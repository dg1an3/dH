        subroutine modify_calc(accessory,lengthx,lengthy,lengthz,mu,
     1                      wedge_angle,tray_factor,ssd,ray)

! *****************************************************************************
!
! Copyright: 1988 Rock Mackie 
!                 University of Wisconsin
!                 Madison, WI
!                            
! Filename: modify_calc.for 
! Function: Produces an accessory matrix that gives the relative absorption
!           through beam modifying accessories.
! Key Words: 
!
! Description: 
!
! Comment for Programmers: Called by CONVOLVE_MAIN.
!                                                  
! Rev#  Date            Name   Revision(s) Made
!
! 0     Nov. 19/86      Rock    Separated from convolve_example.
! 1     May. 17/86      Rock    Parts extracted to block_calc and wedge_calc.
! *****************************************************************************

        implicit none
 
! ********** arguments ********************************************************


! ********** external function references *************************************

        integer wedge_angle

        real accessory(-450:450,-450:450)
        real lengthx,lengthy,lengthz,mu,tray_factor
        real ssd,ray

! ********** global references ************************************************


! ********** local variables **************************************************
             
        integer i,j,block
        character*3 modify 
        character*3 accessory_file,mod_file,store
        character*20 filename_in,filename_out
        character*80 filename2

! *****************************************************************************
                                       

        do i=-50*ray,50*ray                                                 
         do j=-50*ray,50*ray
          accessory(i,j)=1.0
         end do
        end do

2000    format(a20)
7500    format(a3)
                   
        return

        end

                          
