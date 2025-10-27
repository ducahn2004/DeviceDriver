cmd_/home/edoph/Lab1/Ex8/modules.order := {   echo /home/edoph/Lab1/Ex8/stats_provider.ko;   echo /home/edoph/Lab1/Ex8/stats_consumer.ko; :; } | awk '!x[$$0]++' - > /home/edoph/Lab1/Ex8/modules.order
