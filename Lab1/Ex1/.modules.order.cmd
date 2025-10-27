cmd_/home/edoph/Lab1/Ex1/modules.order := {   echo /home/edoph/Lab1/Ex1/hello.ko; :; } | awk '!x[$$0]++' - > /home/edoph/Lab1/Ex1/modules.order
