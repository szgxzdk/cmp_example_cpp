~/cmp_test/cmp_sim_dvfs_RPM/sim-outorder \
        -redir:sim ./sim_result/sp_sim.txt \
        -redir:dump ./sim_result/sp_dump.txt \
	-output:cycle ./sim_result/sp_cycle.txt \
        -config config_mesh_ooo_xy_dvfs \
		-max:barrier 0 -max:inst 10000000 sp.BNC
