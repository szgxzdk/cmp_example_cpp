~/cmp_test/cmp_sim_dvfs_RPM/sim-outorder \
        -redir:sim ./sim_result/hello_sim.txt \
        -redir:dump ./sim_result/hello_dump.txt \
	-output:cycle ./sim_result/hello_cycle.txt \
        -config config_mesh_ooo_xy_dvfs \
		-max:barrier 0 -max:inst 10000000 hello.BNC
