cmd_/home/edoph/Lab1/Ex1/Module.symvers := sed 's/\.ko$$/\.o/' /home/edoph/Lab1/Ex1/modules.order | scripts/mod/modpost    -o /home/edoph/Lab1/Ex1/Module.symvers -e -i Module.symvers   -T -
