
        subroutine density_get(density,lengthx,lengthy,lengthz)

! *****************************************************************************
!
! Copyright: 1988 Rock Mackie
!                 University of Wisconsin
!                 Madison, WI
!                            
! Filename: density_get.for
! Function: Sets up a homogeneous or heterogeneous phantom.
! Key Words:
!
! Description:
!
! Comment for Programmers: Called by CONVOLVE_MAIN.
!
! Rev#  Date            Name            Revision(s) Made
!
! 0     Nov. 23 '86     Rock Mackie     Extracted from convolve_example.for
! 1     June 11 '87     Rock Mackie     Fixed bugs in calculation of k_min
!                                       and k_max
! *****************************************************************************

        implicit none                      

! ********** arguments ********************************************************


! ********** external function references *************************************

        real density(-63:63,-63:63,1:127)
        real lengthx,lengthy,lengthz

! ********** global references ************************************************


! ********** local variables **************************************************

        integer i,j,k,hetstate
        integer i_min,i_max,k_min,k_max,io_i,io_j,io_k
        integer planej

        real homdens,hetdens,breast_radius,xoffset,zoffset
        real x_min,x_max,z_min,z_max,hethick,weightx,weightz
        real posplane,cyl_radius,cyl_length

        character*3 store,file_format,file_unformat
        character*8 now
        character*9 today
        character*20 filename_in,filename_out      
        character*40 filename
        character*80 filename2
        character*80 densityfile_in
        
! *****************************************************************************

! check for reading a density file

        print*,'Do you want to read a density distribution?'
        print*,'Enter "yes" or "no".'   
        read 1000,store
        if (store .eq. 'yes' .or. store .eq. 'YES') then
        
                filename='./../code/coni/format_density.dat'
                
                open(unit=1,
     1                  file='./../code/coni/format_density.dat',
     2                  status='old')

                call format_read(lengthx,            !voxel length in x-dir
     1                   lengthy,            !voxel length in y-dir
     1                   lengthz,            !voxel length in z-dir
     1                   -63,                !min. integer in x-dir
     1                   63,                 !max. integer in x-dir
     1                   1,                  !min. integer in z-dir
     1                   127,                !max. integer in z-dir
     1                   1,                  !planej
     1                   density)            !density
                                                                     
                close(1)
                
                hetstate = 6
                
        else 
                print*,'Using cylindrical phantom (axis perp. to beam)'
                print*,' '
                hetstate = 7
                
        end if 

c Done for a cylidrical phantom perpendicular to the beam direction.
                                                                    
        if (hetstate .eq. 7) then
                               
         print*,' '
         print*,'Enter the radius of the cylinder.'
         read*,cyl_radius                
         print*,' '
         print*,'Enter the length of the cylinder.'
         read*,cyl_length               
         print*,' '
         print*,'Enter the offset of the center of the phantom in x dir'
         print*,'Negative or positive values are valid.'
         read*,xoffset
         print*,' '
         print*,'Enter the offset of the center of the phantom in z dir'
         print*,'Negative or positive values are valid.'
         read*,zoffset
         print*,' '
         print*,'Enter the density of the phantom.'
         read*,hetdens                         
         print*,' '                             
         print*,'Enter the density of the surrounding medium.'
         read*,homdens                                      
         print*,' '
         write(10,*) 'The radius of the cylinder is ...',cyl_radius
         write(10,*) 'The length of the cylinder is ...',cyl_length
         write(10,*) 'The x-offset is ...',xoffset                
         write(10,*) 'The z-offset is ...',zoffset
         write(10,*) 'The cylinder density is ...',hetdens
         write(10,*) 'The surrounding density is ...',homdens
                                      
         do j=-63,63
          do i=-63,63
           do k=1,127
            if (sqrt((float(i)*lengthx-xoffset)**2+
     1            (float(k)*lengthz-zoffset)**2)
     2       .lt. cyl_radius
     3       .and. abs(float(j)*lengthy)
     4       .lt. cyl_length/2.0 ) then
             density(i,j,k)=hetdens
            else
             density(i,j,k)=homdens                      
            end if
           end do
          end do
         end do

        end if

c Write the density file.
        
        print*,'Do you want the density distribution written out?'
        print*,'Enter "yes" or "no".'   
        read 1000,store
        write(10,*)'Is the density distribution written out ? ...',
     1           store                       

        if (store .eq. 'yes' .or. store .eq. 'YES') then
        
         print*,' '                     
         print*,'Do you want to create an formatted file?'
         read 1000,file_format
         write(10,*)'Formatted file ? ...',file_format

         if (file_format .eq. 'yes' .or. file_format .eq. 'YES') then

          print*,' '                             
          print*,'Enter the position (cm) of the plane to view the'
          print*,'distribution. The central axis is at 0.0 cm.'
          read*,posplane
          write(10,*)'The plane the distribution is ...',posplane

          planej=nint(posplane/lengthy)        !determine plane voxel #

c          call date(today)
c          call time(now)           

          filename='./../code/cono/format_density.dat'

          open(unit=1,
     1     file='./../code/cono/format_density.dat',
     2     status='new')

          call format_write(lengthx,            !voxel length in x-dir
     1                   lengthy,            !voxel length in y-dir
     1                   lengthz,            !voxel length in z-dir
     1                   -63,                !min. integer in x-dir
     1                   63,                 !max. integer in x-dir
     1                   1,                  !min. integer in z-dir
     1                   127,                !max. integer in z-dir
     1                   planej,             !integer number for plane
     1                   today,              !today's date
     1                   now,                !time of day
     1                   density,            !density
     1                   filename)           !name of file are writting
                                                                      
          close(1)

         end if

         print*,' '                     
         print*,'Do you want to create an unformatted file?'
         read 1000,file_unformat
         write(10,*)'Unformatted file ? ...',file_unformat
        
         if (file_unformat .eq. 'yes' 
     1                   .or. file_unformat .eq. 'YES')then
                                                                        
          print*,' '
          print*,'Enter the name of the density file.'
          read 2000, filename_out
          write(10,*) ' The name of the stored phantom is ...',
     1              filename_out

          filename2='./../code/cond/'//filename_out

          open(unit=2,file=filename2,status='new',form='unformatted')

          write(2) 127,127,127

          write(2) (((density(i,j,k),i=-63,63),j=-63,63),k=1,127)

          close(2)      
        
         end if
               
        end if

        return

1000    format(a3)
2000    format(a20)

        end

