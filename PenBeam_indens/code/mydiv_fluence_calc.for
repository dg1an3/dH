        subroutine div_fluence_calc

ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c       Copyright (c) 1985, Rock Mackie
c                           Allan Blair Memorial Clinic
c                           Regina, Canada
c
c Key Words: energy fluence 
c
c Function: Calculates the fluence that interacts in the phantom
c           taking into account exponential attenuation, inverse
c           square falloff, heterogeneous phantom density and the
c           divergence of the field boundary.
c
c Filename: div_fluence_calc.for
c Comment for Programmers: Called from CONVOLVE_MAIN. 
c                          The ray startup section and the primary ray-trace
c                          should be extracted and placed in separate modules.
c Revisions:
c  Rev.                 Name of
c   #       Date        Revisor         Revision(s) Made
c
c   1    Nov. 20, 1985  Rock Mackie     Include voxel-by_voxel density.
c   2    Feb. 3, 1990   Dave Convery    Fixed bug in region of interest over
c                                       which to normalize.
c   3    Feb. 10 1990   Rock Mackie     Bug found by Dave Convery fixed. It
c                                       concerned the proper field size at 
c                                       depth.
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

        implicit none

        real thickness,ssd,lengthx,lengthy,lengthz,mindepth
        real xmin,xmax,ymin,ymax
        real incfluence,mu            
        real sourcedep,latscale
        real fieldx,fieldy
        real fieldx_100cm,fieldy_100cm,dist_100cm
        real fluence(-63:63,-63:63,1:127),density(-63:63,-63:63,1:127)
        real accessory(-450:450,-450:450)
        real lensquare,len0,leninc,distance,path
        real pathinc,last_pathinc
        real ro, ray
        real posplane
        real xinc,yinc,fieldx0,fieldy0
        real bottom_scale,x_bottom_min,x_bottom_max
        real y_bottom_min,y_bottom_max
        real offset,horny,trans_factor,max
        real weightx,weighty
        real minfieldx,maxfieldx,minfieldy,maxfieldy
        real delta_path,atten,beam_energy
        real decay,depth , oldfluinc, fluinc 
        real div_scale
        real mvalue, bvalue

        integer i,j,k,depthnum
        integer planej,m,minbndi,maxbndi
        integer inear,jnear
        integer norm(-63:63,-63:63,1:127)
        integer smallfieldi,largefieldi,smallfieldj,largefieldj
        integer imin,imax,jmin,jmax
        integer n,numb_description,numb_records,count

        character*1 beamtype
        character*3 writeflu,file_format,file_unformat
        character*3 horns,geo_penumbra,junk
        character*3 off_axis_soft,depth_soft,transmit
        character*8 now
        character*9 today
        character*40 filename
        character*4 nomen

        common /fluence_common/thickness,
     1       xmin,xmax,ymin,ymax,incfluence,mu,ray,beamtype

        common /all_common/lengthx,lengthy,lengthz,
     1       depthnum,imin,imax,jmin,jmax,fluence,ssd,beam_energy

        common /density/density

        common /accessory_common/accessory

        common /correction/bvalue,mvalue

c added to waive the CF correction for mononergetic beams

         mvalue=0
         bvalue=1
        offset=0.0

        mindepth=ssd-offset+0.5*lengthz   !the depth of the surface voxels.
        
        xinc=lengthx/ray                !the distance between the rays at the
        yinc=lengthy/ray                !phantom surface

        smallfieldi=nint(xmin/xinc)    !integers defining the rays
        largefieldi=nint(xmax/xinc)    
        smallfieldj=nint(ymin/yinc)
        largefieldj=nint(ymax/yinc)

        do i=smallfieldi,largefieldi            !do for the x-direction.

         do j=smallfieldj,largefieldj           !do for the y-direction.

           incfluence=accessory(i,j)

          fieldx0=i*xinc*mindepth/ssd  !these values are the distance from
                                       !the central axis to the ray for
          fieldy0=j*yinc*mindepth/ssd  !rays at the surface voxels.
                                
c Calculate the distance from the focal spot to the ray crossing

          lensquare=fieldx0**2+fieldy0**2+mindepth**2      !initial distance
                                                           !squared
          len0=sqrt(lensquare)           !initial distance
          leninc=len0*lengthz/mindepth   !distance increment; recall that
          distance=len0                  !leninc/len0=lengthz/mindepth

          last_pathinc=0.0      !radiological pathlength in the last voxel
          path=0.0              !radiological pathlength in the phantom
          atten=1.0             !amount of exponential attenuation in the
                                !phantom

c Start raytracing along the ray.

          do k=1,depthnum      !the number of depth voxels

           latscale=1.0+float(k-1)*lengthz/mindepth        !the relative
                                                            !increase in the
                                                            !ray divergence
           fieldx=fieldx0*latscale              !these values are the distance
           fieldy=fieldy0*latscale              !from the central axis to
                                                !the rays.
           inear=nint(fieldx/lengthx)   !the integers describing the
           jnear=nint(fieldy/lengthy)   !nearest voxel

c Calculate the radiological path increment travelled in the phantom.
c                  the factor 0.5 in pathinc is introduced so to have 
c                  smaller steps and account better for inhomogeneities

           pathinc=0.5*leninc*density(inear,jnear,k)
           delta_path=pathinc+last_pathinc   !radiological pathlength increment
           path=path+delta_path
           last_pathinc=pathinc

           weightx=1.0                  !these are the weights if the voxel is
           weighty=1.0                  !completely inside the field.

c The following if statements tests to see if the voxel is sitting on
c the field boundary. If it is the proportion of the voxel inside the
c field is calculated. Dave Convery's second reported bug was fixed 
c by properly scaling the field size for the divergence of the beam.

           div_scale=(ssd+(float(k)-0.5)*lengthz)/ssd
           minfieldx=xmin*div_scale
           maxfieldx=xmax*div_scale
           minfieldy=ymin*div_scale
           maxfieldy=ymax*div_scale
                                  
           if (inear .eq. nint(minfieldx/lengthx)) then
            weightx=float(inear)-minfieldx/lengthx+0.5
           end if

           if (inear .lt. nint(minfieldx/lengthx)) weightx=0.0
        
           if (inear .eq. nint(maxfieldx/lengthx)) then
            weightx=maxfieldx/lengthx-0.5-float(inear-1)
           end if

           if (inear .gt. nint(maxfieldx/lengthx)) weightx=0.0

           if (jnear .eq. nint(minfieldy/lengthy)) then
            weighty=float(jnear)-minfieldy/lengthy+0.5
           end if

           if (jnear .lt. nint(minfieldy/lengthy)) weighty=0.0

           if (jnear .eq. nint(maxfieldy/lengthy)) then
            weighty=maxfieldy/lengthy-0.5-float(jnear-1)
           end if

           if (jnear .gt. nint(maxfieldy/lengthy)) weighty=0.0

c Calculate the relative amount of fluence that interacts in each voxel
c and convert from photons/cm**2 to photons by multiplying by the
c cross-sectional area of the voxel. Also include weighting factors
c that determine the relative number of photons interacting near
c the boundary of the field.

           if (density(inear,jnear,k) .ne. 0.0) then
        
c            nmu=-alog(f_factor(nint(path)))/path

            atten=atten*exp(-mu*delta_path)

            oldfluinc=fluinc                                               

            fluinc=incfluence*atten*mu            !fluence increment    
     2                        *weightx*weighty  
c
c  the divergence correction above was removed from here to be put
c  in new_sphere_convolve

            fluence(inear,jnear,k)=fluence(inear,jnear,k)+fluinc  !fluence

    
            if (fluinc .gt. oldfluinc .and. fluinc .ne. 0) then
                                                      
              continue

                count = count + 1

             end if

            norm(inear,jnear,k)=norm(inear,jnear,k)+1    !normalization

           else

            fluence(inear,jnear,k)=0.0          

            norm(inear,jnear,k)=1               !avoid divide by zero

           end if

           distance=distance+leninc      !distance from the source
                
          end do

100       continue      !break out of depth loop if electron fluence
                        !is accounted for
         end do         
        end do          !close the loops for all the (i,j) voxel pairs

c Normalize the fluence because the voxels had a different number of rays
c passing through them. First have to find out the range through which
c to do the normalization.

        bottom_scale=(ssd+thickness)/ssd        !increase in lateral dimension
                                                !compared to the surface
                                                
c       x_bottom_min=xmin*bottom_scale          !lateral dimensions of bottom
c       x_bottom_max=xmax*bottom_scale          !voxels
c       y_bottom_min=ymin*bottom_scale          !
c       y_bottom_max=ymax*bottom_scale          !

        if (xmin .gt. 0.0) then                 !
            x_bottom_min = xmin                 !
        else                                    ! lateral dimensions of bottom
            x_bottom_min = xmin * bottom_scale  ! voxels
        endif                                   !
        if (xmax .lt. 0.0) then                 !
            x_bottom_max = xmax                 !
        else                                    !
            x_bottom_max = xmax * bottom_scale  !
        endif                                   !
        if (ymin .gt. 0.0) then                 !
            y_bottom_min = ymin                 !
        else                                    !
            y_bottom_min = ymin * bottom_scale  !
        endif                                   !
        if (ymax .lt. 0.0) then                 !
            y_bottom_max = ymax                 !
        else                                    !
            y_bottom_max = ymax * bottom_scale  !
        endif                                   !

        imin=nint(x_bottom_min/lengthx)        !location of bottom voxels
        imax=nint(x_bottom_max/lengthx)        !saves time in normalization
        jmin=nint(y_bottom_min/lengthy)        !below
        jmax=nint(y_bottom_max/lengthy)        !     

        do i=imin,imax
         do j=jmin,jmax
          do k=1,depthnum

           if (norm(i,j,k) .ne. 0) then

            fluence(i,j,k)=fluence(i,j,k)/norm(i,j,k)   !do normalization

           end if

          end do
         end do
        end do

c The following prints out the fluence distribution.

        print*,' '
        print*,'Do you want the energy released dist writ out?'
        print*,'Enter "yes" or "no".'
        read 7500,writeflu
        write(10,*)'Is the energy released distr. written out ? ...',
     1           writeflu

        if (writeflu .eq. 'yes' .or. writeflu .eq. 'YES') then
        
         print*,' '
         print*,'Enter the position (cm) of the plane to view the'
         print*,'distribution. The central axis is at 0.0 cm.'
         read*,posplane
         write(10,*)'The plane the distribution is ...',posplane


         planej=nint(posplane/lengthy) !determine plane voxel #

         print*,' '
         print*,'Do you want to create a formatted file?'
         read 7500,file_format
         write(10,*)'Formatted file ? ...',file_format

         if (file_format .eq. 'yes' .or. file_format .eq. 'YES') then

c          call date(today)
c          call time(now)           

          open(unit=1,
     1     file='./../code/cono/format_fluence.dat',
     2     status='new')

          planej=nint(posplane/lengthy)        !determine plane voxel #

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
     1                   fluence,            !fluence
     1                   filename)           !name of file are writting
                                                                      
         close(1)

         end if


         print*,' '                 
         print*,'Do you want to create an unformatted file?'
         read 7500,file_unformat
         write(10,*) 'Unformatted file ? ...', file_unformat

         if (file_unformat .eq. 'yes' .or. 
     1         file_unformat .eq. 'YES') then

          open(unit=1,
     1       file='./../code/cono/unformat_fluence.dat',
     2       status='new',
     3       form='unformatted')


          write(1) imax-imin+1,depthnum+1

          do k=1,depthnum                       !do for the depth dir.

           write(1) (fluence(i,planej,k)*100.0,
     1                i=imin,imax)

          end do        

          close(1)

         end if

        end if

        close(1)

        return

7000    format(a1)
7500    format(a3)
7600    format(a4)
8000    format(a40)
9000    format(a70)

        end     

