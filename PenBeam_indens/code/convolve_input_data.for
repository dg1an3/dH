
        subroutine convolve_input_data(namefile,!name of formatted report file
     1                       today,          !today's date
     1                       now,            !time of day
     1                       beam_energy,    !the average beam energy/photon
     1                       thickness,      !the maximum thickness of phant.
     2                       lengthx,        !voxel size in x-direction
     3                       lengthy,        !voxel size in y-direction
     4                       lengthz,        !voxel size in z-direction
     5                       ssd,            !source to surface distance
     6                       xmin,           !x-dir smallest field bound.
     7                       xmax,           !x-dir largest field bound.
     8                       ymin,           !y-dir smallest field bound.
     9                       ymax,           !y-dir largest field bound.
     3                       mu,             !nominal attenuation coeff.
     4                       xminroi,        !min. x region of interest
     5                       xmaxroi,        !max. x region of interest
     6                       yminroi,        !min. y region of interest
     7                       ymaxroi,        !max. y region of interest
     8                       zminroi,        !min. z region of interest
     9                       zmaxroi,        !max. z region of interest
     1                       description,    !ascii description of field
     1                       ray)            !number of rays per voxel  
                                              
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c       Copyright (c) 1988 Rock Mackie 
c                          University of Wisconsin
c                          Madison, WI
c                                     
c Key Words: machine parameters
c                                                               
c Function: Inputs data used by the program CONVOLVE_MAIN.FOR
c                                                            
c Filename: convolve_input_data.for
c Comment for Programmers: Called from CONVOLVE_MAIN. Should be 
c                          modified to build a command file that would
c                          automatically repeat the run if desired.
c                                                                  
c Revisions:
c  Rev.                 Name of
c   #       Date        Revisor         Revision(s) Made
c
c   0   Jan 6,   1988  Rock Mackie      Extracted from CONVOLVE_MAIN.FOR
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

        implicit none

        real thickness,ssd,lengthx,lengthy,lengthz
        real xmin,xmax,ymin,ymax,mu
        real xminroi,xmaxroi,yminroi,ymaxroi,zminroi,zmaxroi
        real maxlength,ray,beam_energy
                       
        character*3 file_unformat
        character*8 now
        character*9 today
        character*60 namefile
        character*60 filename2
        character*70 description       

c The output file is chosen. 

        print*,' '
        print*,'Enter the name of the output file.'
        print*,'for example ... results.dat'

        read 8000, namefile
        
        filename2='./../code/cono/'//namefile

        open(unit=10,
     1     file=filename2,
     2     status='new') 

        print*,' '
        print*,'Output file chosen is ...'
        print*,filename2
        print*,' '
        
c Write out the name of the output file and the time and date.

c        call date(today)
c        call time(now)

        write(10,*)'This file is ... '
        write(10,*) filename2               
        write(10,*)
        write(10,*)'It was written on ... ',today,' ',now  
        write(10,*)
        
c Input some beam parameters.

        print*,' '
        print*,'Enter the average primary photon energy (MeV).'
        read*,beam_energy
        print*,' '

        print*,' '
        print*,'Enter the phantom thickness (cm).'
        read*,thickness
        print*,' '
                                                                  
        continue
        print*,'Enter the voxel dimension in the x-direction (cm).'
        read*,lengthx
        print*,' '
        print*,'Enter the voxel dimension in the y-direction (cm).'
        read*,lengthy
        print*,' '
100     print*,'Enter the voxel dimension in the z-direction (cm).'
        read*,lengthz
        print*,' '                                              
   
        if (thickness/lengthz .gt. 127.0) then
         maxlength=thickness/127.0
         print*,'Too many voxels in the depth direction.'
         print*,'The maximum size is...',maxlength 
         print*,'Increase the voxel dimension in the z-direction.'
         print*,' '
         goto 100
        end if

        print*,'Enter the source-to-phantom surface distance (cm).'      
        read*,ssd                                 
        print*,' '

        print*,'Enter the field size at the surface in the x-direction.'
        print*,'The program models independent collimator jaws.'
        print*,'The smaller value is the left field boundary'
        print*,'and the larger value is the right field boundary.'
        print*,'with respect to the central axis. Both values can'
        print*,'be negative or positive.'
        print*,'Smaller field, Larger field (cm).'
        read*,xmin,xmax
        print*,' '

        print*,'Enter the field size at the surface in the y-direction.'
        print*,'The same comments for the x-direction apply here.'
        print*,'Smaller field, Larger field (cm).'
        read*,ymin,ymax
        print*,' '

        print*,'Enter the mass attenuation coefficient' 
        print*,'corresponding to the primary energy (cm**2/g).'
        read*,mu
        print*,' '                                            
        
        write(10,*)'The average primary photon energy is',
     1           beam_energy,' MeV'
        write(10,*)'The water thickness is ...',thickness,' cm'
        write(10,*)'The voxel dim. in x-direction is ..',lengthx,' cm'
        write(10,*)'The voxel dim. in y-direction is ..',lengthy,' cm'
        write(10,*)'The voxel dim. in z-direction is ..',lengthz,' cm'
        write(10,*)'The ssd is .........',ssd,' cm'
        write(10,*)'The field in x-dir. is ...',xmin,' to ',xmax,' cm'
        write(10,*)'The field in y-dir. is ...',ymin,' to ',ymax,' cm'
        write(10,*)'The nominal primary atten. coeff. is ..',mu,' cm2/g'

c Enter the region of interest where the dose will be calculated.
                                    
        print*,'Enter the region where the dose is to be calculated.'
        print*,' '
        print*,'Enter the region of interest in the 
     1          x direction.'
        print*,'The region of interest can be inside or outside the 
     1          field.'
        print*,'Smaller value, Larger value (cm).'
        print*,'eg. for central axis depth dose enter ... 0.0,0.0' 
        read*,xminroi,xmaxroi
        print*,' '

        print*,'Enter the region of interest in the 
     1          y direction.'
        print*,'The region of interest can be inside or outside the 
     1          field.'
        print*,'Smaller value, Larger value (cm).'
        print*,'eg. for central axis depth dose enter ... 0.0,0.0' 
        read*,yminroi,ymaxroi
        print*,' '

        print*,'Enter the region of interest in the depth-direction.'
        print*,'The smaller value must be > or = 0.0'
        print*,'The larger value must be < or = phantom thickness'
        print*,'Smaller value, Larger value (cm).'
        print*,'eg. for profile at 10 cm depth enter ... 10.0,10.0' 
        read*,zminroi,zmaxroi
        print*,' '                  
                          
        write(10,*)'The x reg. of interest is..',xminroi,' to ',
     1               xmaxroi,' cm'
        write(10,*)'The y reg. of interest is..',yminroi,' to ',
     1               ymaxroi,' cm'
        write(10,*)'The depths of interest is..',zminroi,' to ',
     1               zmaxroi,' cm'
        print*,'If desired enter a general description of the field.'
        print*,'Enter a maximum of 70 characters.' 
        read 9000, description

        print*,' '              
        write(10,*)' '
        write(10,*)description
        write(10,*)' '
                                                                                
        print*,'Enter the number of rays per voxel 
     1       at the surface (1 to 9)'
        read*,ray
        write(10,*)'Number of rays per voxel at the surface...',ray
        print*,' '
        
8000    format( a60 )
9000    format( a70 )

        return
        end
