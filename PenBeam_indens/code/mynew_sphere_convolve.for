        subroutine new_sphere_convolve 

ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c       Copyright (c) 1986, 1987, 1988, 1989 Rock Mackie 
c                                            University of Wisconsin
c                                            Madison, WI
c                                            
c Key Words: convolution, spherical dose spread arrays 
c
c Function: Convolves spherical dose spread arrays in a cartesian 
c           energy deposition 2-dimensional matrix. It requires that
c           an input file be created by program SUM_ELEMENTS .
c
c Filename: new_sphere_convolve.for
c Comment for Programmers: Called from CONVOLVE_MAIN.
c Revisions:                          
c  Rev.                 Name of
c   #       Date        Revisor         Revision(s) Made
c   1   May 7, 1986     Rock Mackie     Added capability to handle
c                                       density 
c   2   May 11, 1987    Rock Mackie     Added voxel dimensions in each
c                                       direction
c   3   Jan 5,  1988    Rock Mackie     3-D region of interest possible
c   4   Mar 1,  1988    Rock Mackie     Changed ray-trace calculation
c   5   Aug 5,  1989    Rock Mackie     Allow input of arbitrary numbers of
c                                       zenith and azimuthal ray-trace 
c                                       directions               
c   6   Nov 19, 1989    Rock Mackie     Implemented the use of a faster
c                                       lookup table that avoids having
c                                       to interpolate
c   7   May 29, 1992    Nikos Papanikolaou
c                                       Divergence correction moved here
c                                       from the div_fluence_calc
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

        implicit none

        integer i,j,k,mini,maxi,minj,maxj,mink,maxk,a,r
        integer phi,thet,numthet,numphi,numrad
        integer delk,deli,delj         
        integer imin,imax,jmin,jmax
        integer depthnum,dmaxi,dmaxj,dmaxk
        integer includ,exclud             
        integer numstep,rad_inc
        integer delta_i(64,48,48),delta_j(64,48,48),delta_k(64,48,48)
                                                              
        real pi,inc_energy(48,0:48),ang(48)
c The new_distance added by Nikos to be used for the divergence 
        real lengthx,lengthy,lengthz,new_distance       
        real trigx,trigy,trigz,triglat,radx,rady,radz,minrad
        real energy_out(-63:63,-63:63,1:127)
        real energy_out2(-63:63,-63:63,1:127)
        real fluence(-63:63,-63:63,1:127)
        real density(-63:63,-63:63,1:127)
        real max,time,pathinc
        real sample_fraction,inck,inci,incj,sumi,sumj,sumk,rad_dist
        real delta_rad
        real lat_pos_square,spd,lat_pos,ssd
        real cos_alpha,sin_alpha,alpha
        real cos_beta,sin_beta,beta
        real new_angle,min,diff
        real radius(0:64,48,48),rad_bound(0:48)
        real tot_energy,last_energy,energy
        real last_rad,temp,avdens,beam_energy,kerndens
        real cum_energy(48,0:600)
c depth_distance added for the CF kernel depth hardening effect
        real mvalue,bvalue,c_factor,depth_distance
        real dummy
                                             
        character*4 choose
        character*3 pop,normalize,ray_trace
        character*25 namefile
        character*80 filename1

        common /all_common/lengthx,lengthy,lengthz,
     1       depthnum,imin,imax,jmin,jmax,fluence,ssd,beam_energy

        common /convolve_common/mini,maxi,mink,maxk,minj,maxj,
     1       energy_out,dmaxi,dmaxj,dmaxk,max,time

        common /enter/filename1,choose,includ,exclud
                              
        common /density/density

        common /correction/bvalue,mvalue

        print*,' '      
        print*,'Do you want to do voxel-by-voxel ray-tracing?'
        print*,'i.e. Do you want to do superposition?'
        print*,'Enter "yes" for superposition'
        print*,'...or "no" for convolution.'                        
        read 7500,ray_trace

        if (ray_trace .eq. 'no' .or. ray_trace .eq. 'NO') then

         print*,' '
         print*,'You are not doing voxel-by-voxel ray-tracing.'
         print*,'Enter the average density of the phantom.'          
         read*,avdens
         
         write(10,*)'Convolution calculation was done'           
         write(10,*)'with an average density of:',avdens
        
        else

         write(10,*)'Superposition calculation was done.'           
                                                        
        end if

        pi=3.141593
        sample_fraction=0.5     !fraction of voxel size to use as the step size
        numstep=64
         
        print*,' '
        print*,'Enter the number of azimuthal directions to rt over.'
        print*,'Enter an integer value. For example, 8 or 16.'
        read*,numthet                       
        
        write(10,*)'The number of azimuthal ray-trace directions was',
     1    numthet
                                                                             
c The file to read is chosen.

        print*,' '
        print*,'Enter the name of dose spread array file'
        print*,'with the elements summed.'
        print*, 'For example ... nrg1250tot.dat'
        read 8000, namefile                   
        write(10,*)'Kernel file name ...', namefile

        filename1='./../code/coni/'//namefile

        print*,' '
        print*,'Enter the density of the kernel that is used for'
        print*,'the computation of dose'
        read*,kerndens                      
        
        write(10,*)'The kernel density is',kerndens

        open(unit=1,
     1    file=filename1,
     2    status='old')

        print*,' '
        print*,'Input file chosen is ...'
        print*,filename1


c The dose spread arrays produced by SUM_ELEMENT.FOR are read.
                                                    
        read (1,*)
        read (1,*)
        read (1,*)
        read (1,*) numphi       !number of angular divisions
        read (1,*) numrad       !number of radial divisions
        read (1,*)
        read (1,*)
        read (1,*)
        read (1,*)

        do a=1,numphi

         read (1,*) (inc_energy(a,r),r=1,numrad) !read dose spread values 
                     
        end do

        do a=1,numphi
               
         inc_energy(a,0)=0.0

         do r=1,numrad
         
          inc_energy(a,r)=inc_energy(a,r)+inc_energy(a,r-1)
                                        !read dose spread values 

         end do                                       

        end do

        read (1,*)
        read (1,*)
        read (1,*)

        do a=1,numphi
        
         read (1,*) ang(a)      !read mean angle of spherical voxels 

        end do

        read (1,*)
        read (1,*)
        read (1,*)

        rad_bound(0)=0.0

        do r=1,numrad           
              
         read (1,*) rad_bound(r)   !read radial boundaries of spherical voxels
                                                        
        end do
        
c Routine sets up the ray-trace calculation.
                                                
        call ray_trace_set_up           (numphi, 
     1                               numthet,
     1                               numstep,
     1                               ang, 
     1                               lengthx,
     1                               lengthy,
     1                               lengthz,
     1                               radius,
     1                               delta_i,
     1                               delta_j,
     1                               delta_k)        



        do a=1,numphi
                     
         call energy_lookup(rad_bound,
     1                   numrad,
     1                   a,
     1                   inc_energy,
     1                   cum_energy)
              
        end do
                                                                     
c Do the convolution.

        time=0.0 !secnds(0.0)        !begin elapsed time

        do k=mink,maxk          !do for region of interest in z-dir
                  
         do j=minj,maxj         !do for region of interest in y-dir

          do i=mini,maxi        !do for region of interest in x-dir
                                                                   
           if (density(i,j,k) .eq. 0.0) goto 2000       !dose at zero density
                                                        !voxel is meaningless

           do thet=1,numthet            !do for all azimuthal angles

            do phi=1,numphi             !do for zenith angles 
                               
             last_rad=0.0
             rad_dist=0.0
             last_energy=0.0

             do rad_inc=1,numstep       !loop over radial increments
                         
              deli=delta_i(rad_inc,phi,thet)    !integer distances between
              delj=delta_j(rad_inc,phi,thet)    !the interaction and
              delk=delta_k(rad_inc,phi,thet)    !the dose depostion voxels
                                                                          
              if (k-delk .lt. 1 .or.        !test to see if inside field
     1         k-delk .gt. depthnum .or. !and phantom
     1         i-deli .gt. 63 .or.
     1         i-deli .lt. -63 .or.
     1         j-delj .gt. 63 .or.
     1         j-delj .lt. -63) goto 1000 
                  
c Increment rad_dist, the radiological path from the dose deposition site. 
              
              pathinc=radius(rad_inc,phi,thet)
                                     
              if (ray_trace .eq. 'yes' .or. ray_trace .eq. 'YES') then

c changed by nikos (Sept.12 93) from:
c              delta_rad=pathinc*density(i-deli,j-delj,k-delk)
c  to....:
               delta_rad=pathinc*density(i-deli,j-delj,k-delk)/kerndens
c              delta_rad=pathinc
              else

c changed by nikos (Sept.12 93) from:
c              delta_rad=pathinc*avdens
c  to....:
               delta_rad=pathinc*avdens/kerndens
c              delta_rad=pathinc

              end if

              rad_dist=rad_dist+delta_rad

              if (rad_dist .ge. 60) goto 1000    !for high density medium
c                                                the radiological path
c                                               exceeds the kernel size
c                                              which is 60cm by definition

c Use lookup table to find the value of the cumulative energy
c deposited up to this radius. No interpolation is done. The
c resolution of the array in the radial direction is every mm
c hence the multiply by 10.0 in the arguement.
                                                        
              tot_energy=cum_energy(phi,nint(rad_dist*10.0))

c Subtract the last cumulative energy from the new cumulative energy
c to get the amount of energy deposited in this interval and set the
c last cumulative energy for the next time the lookup table is used.
                                                                    
              energy=tot_energy-last_energy
              last_energy=tot_energy                     

c The energy is accumulated.
                            
              energy_out(i,j,k)=energy_out(i,j,k)
     1               +energy*fluence(i-deli,j-delj,k-delk)  !superposition
              
50            continue

             end do             !end of radial path loop                

1000         continue 

            end do       !end of azimuth angle loop 
           end do       !end of zenith angle loop

2000       dummy=0.0
           end do        !end of z-direction loop  
         end do         !end of y-direction loop
        end do          !end of x-direction loop 
        
c Normalize the energy deposited to take into account the
c summation over the azimuthal angles. Convert the energy to  
c dose by dividing by mass and find the position of dmax
c and the dose at dmax.                              

        max=0.0

        do k=mink,maxk
         do j=minj,maxj
          do i=mini,maxi      


c The divergence correction is done also here (instead in div_fluence_calc)
c Added by Nikos

             new_distance=sqrt((ssd+float(k)*lengthz-0.5)**2+
     1                 (float(j)*lengthy)**2+
     2                 (float(i)*lengthx)**2)

             depth_distance=sqrt((float(k)*lengthz-0.5)**2+
     1                 (float(j)*lengthy)**2+
     2                 (float(i)*lengthx)**2)

             c_factor=mvalue*depth_distance+bvalue

           energy_out2(i,j,k)=(energy_out(i,j,k)*1.602e-10/numthet)*
     1                     (ssd/new_distance)**2*
     2                     c_factor
                                                      !convert to Gy cm**2      
                                                      !and take into
                                                      !account the
                                                      !azimuthal sum
           if (energy_out2(i,j,k) .gt. max) then               
        
            max=energy_out2(i,j,k)       
            dmaxi=i                     !position of
            dmaxj=j                     !dmax
            dmaxk=k                                                             

           end if

          end do
         end do
        end do

c Normalize the dose to the dose at dmax.

        print*,' '
c print*,'Do you want the dose normalized to dmax?'
c print*,'Enter "yes" or "no".'
c read 7500,normalize
        normalize = 'yes'

        open(unit=1,
     1    file='./../code/cono/max.dat',
     2    status='new')

        write(1,*) max
        close(1)

        write(10,*)'Dose normalized to dmax ? ...', normalize

        if (normalize .eq. 'yes' .or. 
     1   normalize .eq. 'YES') then

         if (max .gt. 0.0) then

          do k=mink,maxk
           do j=minj,maxj
            do i=mini,maxi  

             energy_out2(i,j,k)=energy_out2(i,j,k)/max
                
            end do
           end do
          end do 

         end if

        end if

c It is very easy to simulate opposed beams.
                                            
c print*,'Do you want opposed beams?'
c read 7500,pop     
        pop = 'no'

        write(10,*)'Parallel opposed pair ? ...', pop

        do k=mink,maxk
         do j=minj,maxj
          do i=mini,maxi

           if (pop .eq. 'yes' .or. pop .eq. 'YES') then

            energy_out(i,j,k)=(energy_out2(i,j,k)+       !parallel opposed
     1                 energy_out2(i,j,maxk-k+mink))  !pair calculated
                                                                         
           else

            energy_out(i,j,k)=energy_out2(i,j,k)     !single field

           end if

          end do
         end do
        end do
                                                                           
7500    format(a3)
8000    format(a25)
c10000   format(' ',10(1pe9.3,1x))
                  
        return
        end

