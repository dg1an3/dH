                        
        subroutine format_write(lengthx,        !voxel length in x-dir
     1                   lengthy,            !voxel length in y-dir
     1                   lengthz,            !voxel length in z-dir
     1                   imin,               !min. integer in x-dir
     1                   imax,               !max. integer in x-dir
     1                   kmin,               !min. integer in z-dir
     1                   kmax,               !max. integer in z-dir
     1                   planej,             !integer number for plane
     1                   today,              !today's date
     1                   now,                !time of day
     1                   value,              !data that is being written
     1                   filename)           !name of file are writing
                                                                     
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c This is a modification of math_write.f
c
c       Copyright (c) 1988 Rock Mackie 
c                          University of Wisconsin
c                          Madison, WI
c                                     
c Key Words: machine parameters, energy fluence
c                                              
c Function: Writes out a formatted 2-D array. 
c
c Filename: format_write.for
c Comment for Programmers: Called by DIV_FLUENCE_CALC and DENSITY_GET. 
c                          The values must be between 0.0 and 9.9999...
c
c Revisions:
c  Rev.                 Name of
c   #       Date        Revisor         Revision(s) Made               
c
c   0   Jan 8, 1988     Rock Mackie     Taken from DIV_FLUENCE_CALC.FOR
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

        implicit none
                                      
        real lengthx,lengthy,lengthz,value(-63:63,-63:63,1:127)
        
        integer i,k
        integer imin,imax,kmin,kmax,planej

        character*8 now
        character*9 today
        character*40 filename


         do k=kmin,kmax                 !do for the depth dir.
          write(1,*)' '
          do i=imin,imax                 !do for the x-dir.

          write(1,*)  value(i,planej,k)

          end do
         end do

        return

1000    format(' ',f5.2,3x,1pe10.3,3x,f5.2)
        
        end
