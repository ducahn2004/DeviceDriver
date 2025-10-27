cmd_/home/edoph/Lab1/Ex8/Module.symvers := sed 's/\.ko$$/\.o/' /home/edoph/Lab1/Ex8/modules.order | scripts/mod/modpost    -o /home/edoph/Lab1/Ex8/Module.symvers -e -i Module.symvers   -T -
