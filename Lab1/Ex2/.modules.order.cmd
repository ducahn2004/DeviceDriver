cmd_/home/edoph/Lab1/Ex2/modules.order := {   echo /home/edoph/Lab1/Ex2/hello.ko; :; } | awk '!x[$$0]++' - > /home/edoph/Lab1/Ex2/modules.order
