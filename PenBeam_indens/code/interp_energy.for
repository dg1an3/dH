        subroutine interp_energy(bound_1,       !inner boundary
     1                        bound_2,       !outer boundary
     1                        rad_numb,      !radial label of inner voxel
     1                        phi_numb,      !angular label of voxel
     1                        radius,        !radius of voxel boundaries
     1                        inc_energy,    !array of incremental energy
     1                        energy)        !energy between boundaries  

ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c       Copyright (c) 1988, Rock Mackie 
c                           University of Wisconsin
c                           Madison, WI
c                                      
c Key Words: convolution, spherical dose spread arrays 
c                                      
c Function: interpolates to find the incremental energy deposited 
c           up to a point.
c                                                                   
c Filename: interp_energy.for                                           
c Comment for Programmers: Called by NEW_SPHERE_CONVOLVE.
c Revisions:
c  Rev.                 Name of
c   #       Date        Revisor         Revision(s) Made
c   0   Feb 3, 1988     Rock Mackie     First written
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

        implicit none
                           
        integer rad_numb,
     1       phi_numb 
                                                          
        real    bound_1,
     1       bound_2,  
     1       inc_energy1,
     1       inc_energy2,
     1       inc_energy(48,0:48),
     1       radius(0:48),      
     1       energy
                                 
        inc_energy1=inc_energy(phi_numb,rad_numb-1)
        inc_energy2=inc_energy(phi_numb,rad_numb)
                        
        energy=inc_energy1+(inc_energy2-inc_energy1)
     1               *(bound_2-radius(rad_numb-1))  
     1               /(radius(rad_numb)-radius(rad_numb-1))
                          
        return
        
        end
