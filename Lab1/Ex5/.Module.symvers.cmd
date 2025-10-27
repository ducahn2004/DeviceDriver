cmd_/home/edoph/Lab1/Ex5/Module.symvers := sed 's/\.ko$$/\.o/' /home/edoph/Lab1/Ex5/modules.order | scripts/mod/modpost    -o /home/edoph/Lab1/Ex5/Module.symvers -e -i Module.symvers   -T -
