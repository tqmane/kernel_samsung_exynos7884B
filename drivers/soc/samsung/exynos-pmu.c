/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS - CPU PMU(Power Management Unit) support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/smp.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>
#include <linux/platform_device.h>
#include <asm/smp_plat.h>
#include <soc/samsung/exynos-pmu.h>

/**
 * "pmureg" has the mapped base address of PMU(Power Management Unit)
 */
static struct regmap *pmureg;

/**
 * No driver refers the "pmureg" directly, through the only exported API.
 */
int exynos_pmu_read(unsigned int offset, unsigned int *val)
{
	return regmap_read(pmureg, offset, val);
}

int exynos_pmu_write(unsigned int offset, unsigned int val)
{
	return regmap_write(pmureg, offset, val);
}

int exynos_pmu_update(unsigned int offset, unsigned int mask, unsigned int val)
{
	return regmap_update_bits(pmureg, offset, mask, val);
}

struct regmap *exynos_get_pmu_regmap(void)
{
	return pmureg;
}

EXPORT_SYMBOL_GPL(exynos_get_pmu_regmap);
EXPORT_SYMBOL(exynos_pmu_read);
EXPORT_SYMBOL(exynos_pmu_write);
EXPORT_SYMBOL(exynos_pmu_update);

/**
 * CPU power control registers in PMU are arranged at regular intervals
 * (interval = 0x8). pmu_cpu_offset calculates how far cpu is from address
 * of first cpu. This expression is based on cpu and cluster id in MPIDR,
 * refer below.

 * cpu address offset : ((cluster id << 2) | (cpu id)) * 0x8
 */

/*#ifdef CONFIG_SOC_EXYNOS9810
#define CPU_PER_OFFSET		0x8
#define phy_cluster(cpu)	!MPIDR_AFFINITY_LEVEL(cpu_logical_map(cpu), 1)
#else
#define CPU_PER_OFFSET		0x80
#define phy_cluster(cpu)	MPIDR_AFFINITY_LEVEL(cpu_logical_map(cpu), 1)
#endif

#define phy_cpu(cpu)	MPIDR_AFFINITY_LEVEL(cpu_logical_map(cpu), 0)

#define pmu_cpu_offset(cpu)	(((phy_cluster(cpu) << 2)| phy_cpu(cpu)) * CPU_PER_OFFSET)
*/
#define phy_cluster(cpu)        MPIDR_AFFINITY_LEVEL(cpu_logical_map(cpu), 1)
#define PMU_CPU_CONFIG_BASE			0x2000
#define PMU_CPU_STATUS_BASE			0x2004
#define CPU_LOCAL_PWR_CFG			0xF

#define CLUSTER0_CORES_CNT      (2)
#define CLUSTER2_CORES_CNT      (2)
unsigned int pmu_cpu_offset(unsigned int mpidr)
{
        unsigned int cluster = MPIDR_AFFINITY_LEVEL(mpidr, 1);
        unsigned int offset = 0;
        unsigned int cpuid = 0;

        // It's base on the PMU's offset.
        switch (cluster) {
        case 1:
                cpuid += CLUSTER2_CORES_CNT;
        case 2:
                cpuid += CLUSTER0_CORES_CNT;
        case 0:
                cpuid += MPIDR_AFFINITY_LEVEL(mpidr, 0);
                break;
        }

        offset = 0x80 * cpuid;

        return offset;
}

static void pmu_cpu_ctrl(unsigned int cpu, int enable)
{
	unsigned int offset;
	unsigned int mpidr = cpu_logical_map(cpu);
	offset = pmu_cpu_offset(mpidr);
	regmap_update_bits(pmureg, PMU_CPU_CONFIG_BASE + offset,
			CPU_LOCAL_PWR_CFG,
			enable ? CPU_LOCAL_PWR_CFG : 0);
}

static int pmu_cpu_state(unsigned int cpu)
{
	unsigned int offset, val = 0;
	unsigned int mpidr = cpu_logical_map(cpu);
	offset = pmu_cpu_offset(mpidr);
	regmap_read(pmureg, PMU_CPU_STATUS_BASE + offset, &val);

	return ((val & CPU_LOCAL_PWR_CFG) == CPU_LOCAL_PWR_CFG);
}

#define CLUSTER_ADDR_OFFSET			0x20
#define PMU_CPUSEQ_OPTION_BASE			0x2488
#define PMU_NONCPU_STATUS_BASE			0x2404
#define PMU_L2_STATUS_BASE			0x2604
#define NONCPU_LOCAL_PWR_CFG			0xF
#define L2_LOCAL_PWR_CFG			0x7

static void pmu_cluster_ctrl(unsigned int cpu, int enable)
{
	unsigned int offset;

	offset = phy_cluster(cpu) * CLUSTER_ADDR_OFFSET;
	regmap_update_bits(pmureg,
			PMU_CPUSEQ_OPTION_BASE + offset,
			1,enable);
}

static bool pmu_noncpu_state(unsigned int cpu)
{
	unsigned int noncpu_stat = 0;
	unsigned int offset;

	offset = phy_cluster(cpu) * CLUSTER_ADDR_OFFSET;
	regmap_read(pmureg,
		PMU_NONCPU_STATUS_BASE + offset, &noncpu_stat);

	return !!(noncpu_stat & NONCPU_LOCAL_PWR_CFG);
}

static int pmu_l2_state(unsigned int cpu)
{
	unsigned int shared_stat = 0;
	unsigned int offset;

	offset = phy_cluster(cpu) * CLUSTER_ADDR_OFFSET;

	regmap_read(pmureg,
		PMU_L2_STATUS_BASE + offset, &shared_stat);

	return (shared_stat & L2_LOCAL_PWR_CFG);
}

static void exynos_cpu_up(unsigned int cpu)
{
	pmu_cpu_ctrl(cpu, 1);
}

static void exynos_cpu_down(unsigned int cpu)
{
	pmu_cpu_ctrl(cpu, 0);
}

static int exynos_cpu_state(unsigned int cpu)
{
	return pmu_cpu_state(cpu);
}

static void exynos_cluster_up(unsigned int cpu)
{
	pmu_cluster_ctrl(cpu, false);
}

static void exynos_cluster_down(unsigned int cpu)
{
	pmu_cluster_ctrl(cpu, true);
}

static int exynos_cluster_state(unsigned int cpu)
{
	return pmu_l2_state(cpu) &&
			pmu_noncpu_state(cpu);
}

struct exynos_cpu_power_ops exynos_cpu = {
	.power_up = exynos_cpu_up,
	.power_down = exynos_cpu_down,
	.power_state = exynos_cpu_state,
	.cluster_up = exynos_cluster_up,
	.cluster_down = exynos_cluster_down,
	.cluster_state = exynos_cluster_state,
};

#define PMU_CP_STAT		0x0038
int exynos_check_cp_status(void)
{
	unsigned int val;

	exynos_pmu_read(PMU_CP_STAT, &val);

	return val;
}

#define PMU_CPU_RESET_BASE		(0x200C)
#define DISABLE_WDT_CPUPORESET		BIT(12)
#define DISABLE_CORERESET		BIT(9)
#define DISABLE_CPUPORESET		BIT(8)
#define DISABLE_RESET			(DISABLE_WDT_CPUPORESET	\
					| DISABLE_CORERESET 	\
					| DISABLE_CPUPORESET)
void exynos_cpu_reset_enable(unsigned int cpu)
{
	unsigned int mpidr = cpu_logical_map(cpu);
	unsigned int offset;

	offset = pmu_cpu_offset(mpidr);
	regmap_update_bits(pmureg, PMU_CPU_RESET_BASE + offset,
			DISABLE_RESET, 0);
}

void exynos_cpu_reset_disable(unsigned int cpu)
{
	unsigned int mpidr = cpu_logical_map(cpu);
	unsigned int offset;

	offset = pmu_cpu_offset(mpidr);
	regmap_update_bits(pmureg, PMU_CPU_RESET_BASE + offset,
			DISABLE_RESET, DISABLE_RESET);
}

static struct bus_type exynos_info_subsys = {
	.name = "exynos_info",
	.dev_name = "exynos_info",
};

static ssize_t core_status_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	ssize_t n = 0;
	int cpu = 0;

	for_each_possible_cpu(cpu) {
		unsigned int mpidr = cpu_logical_map(cpu);
		unsigned int phy_cpu = MPIDR_AFFINITY_LEVEL(mpidr, 0);

		if (!phy_cpu) {
			n += scnprintf(buf + n, 24, "%s L2 : %d\n",
				(!cpu) ? "boot" : "nonboot",
				pmu_l2_state(cpu));

			n += scnprintf(buf + n, 24, "%s Noncpu : %d\n",
				(!cpu) ? "boot" : "nonboot",
				pmu_noncpu_state(cpu));
		}
		n += scnprintf(buf + n, 24, "CPU%d : %d\n",
				cpu, pmu_cpu_state(cpu));
	}

	return n;
}

static struct kobj_attribute cs_attr =
	__ATTR(core_status, 0644, core_status_show, NULL);

static struct attribute *cs_sysfs_attrs[] = {
	&cs_attr.attr,
	NULL,
};

static struct attribute_group cs_sysfs_group = {
	.attrs = cs_sysfs_attrs,
};

static const struct attribute_group *cs_sysfs_groups[] = {
	&cs_sysfs_group,
	NULL,
};

#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/cpu.h>

static int exynos_pmu_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	//debug statements
	printk("%s started",__func__);
	pmureg = syscon_regmap_lookup_by_phandle(dev->of_node,
						"samsung,syscon-phandle");
	if (IS_ERR(pmureg)) {
		pr_err("Fail to get regmap of PMU\n");
		return PTR_ERR(pmureg);
	}

	if (subsys_system_register(&exynos_info_subsys,
					cs_sysfs_groups))
		pr_err("Fail to register exynos_info subsys\n");
	/*FIXME uncomment during hotplug feature*/
	//register_cpus_notifier(&exynos_pmu_cpus_nb);
	printk("%s completed",__func__);
	return 0;
}

static const struct of_device_id of_exynos_pmu_match[] = {
	{ .compatible = "samsung,exynos-pmu", },
	{ },
};

static const struct platform_device_id exynos_pmu_ids[] = {
	{ "exynos-pmu", },
	{ }
};

static struct platform_driver exynos_pmu_driver = {
	.driver = {
		.name = "exynos-pmu",
		.owner = THIS_MODULE,
		.of_match_table = of_exynos_pmu_match,
	},
	.probe		= exynos_pmu_probe,
	.id_table	= exynos_pmu_ids,
};

int __init exynos_pmu_init(void)
{
	return platform_driver_register(&exynos_pmu_driver);
}
subsys_initcall(exynos_pmu_init);
