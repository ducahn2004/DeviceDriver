cmd_/home/edoph/Lab1/Ex5/modules.order := {   echo /home/edoph/Lab1/Ex5/hello_multi.ko; :; } | awk '!x[$$0]++' - > /home/edoph/Lab1/Ex5/modules.order
