type Input.Big_6mv\Input%1.txt

convolve_main < Input.Big_6mv_cylinder\Input%1.txt

mkdir cono\output%1
move cono\format_dose.dat cono\output%1
move cono\format_fluence.dat cono\output%1
move cono\inputLog%1 cono\output%1
del cono\max.dat
del cono\format_density.dat