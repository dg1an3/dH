        subroutine make_vector(numstep,         !number of voxels passed thru
     1                      factor1,         !principle direction cosine
     1                      factor2,         !secondary
     1                      factor3,         !   direction cosines
     1                      r,               !list of lengths thru a voxel
     1                      delta1,          !location in principle direction
     1                      delta2,          !locations in
     1                      delta3)          !   secondary directions
                                                                        
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c       Copyright (c) 1988, Rock Mackie
c                           University of Wisconsin
c                           Madison, WI
c
c Key Words: ray-trace, boundary crossing
c                                        
c Function: Makes a list of radial lengths to a defined principal plane that
c           is used for ray-tracing. 
c
c Filename: make_vector
c Comment for Programmers: Called by RAY_TRACE_SET_UP. The vector starts
c                          at the center of a voxel and goes in a direction
c                          in 3-space.
c Revisions:                                          
c  Rev.                 Name of
c   #       Date        Revisor         Revision(s) Made
c   0   Feb. 4, 1988    Rock Mackie     Program written.
c   1   June. 14, 1988  Rock Mackie     More documentation 
c   2   Jan. 27, 1989   Rock Mackie     Still more documentation
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

        implicit none

        integer i,numstep,delta1(64),delta2(64),delta3(64)

        real d,r(64),factor1,factor2,factor3

c At each ray-trace step determine the radius and the voxel number 
c along each coordinate direction.                                 

        do i=1,numstep
        
         d=float(i)-0.5                 !distance to the end of a voxel
                                                                      
         if (factor1 .lt. 0.0) d=-d     !want absolute distance
                                                               
         if (abs(factor1) .ge. 1e-04) then
        
          r(i)=d/factor1                !radius to interection point
                
         else
        
          r(i)=100000.0                 !effectively infinity

         end if                              

c Calculate a distance along a coordinate direction and find the nearest
c integer to specify the voxel direction.
        
         delta1(i)=nint(0.99*r(i)*factor1)  !0.99 prevents being exactly on
                                            !the voxel boundary
         delta2(i)=nint(r(i)*factor2)       
                                            
         delta3(i)=nint(r(i)*factor3)       
                                            
        end do
        
        return

        end
