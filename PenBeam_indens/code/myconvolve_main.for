       program convolve_main                                                  

ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c 
c       Copyright (c) 1985, 1986, 1987, 1988, 1989 Rock Mackie 
c                                                  Department of Medical Physics
c                                                  University of Wisconsin
c                                                  Madison, WI
c                                                       
c Key Words: machine parameters, convolution, spherical dose spread arrays, 
c            fluence
c
c Function: Calculates the amount of energy deposited in a water-like 
c           phantom in a cartesian geometry using superposition. 
c
c Filename: convolve_main.for
c Comment for Programmers: This program convolves by sampling the energy
c                          spreading out in a spherical geometry. 
c
c Revisions:
c  Rev.                 Name of
c   #       Date        Revisor         Revision(s) Made
c
c   1   Nov. 19, 1985  Rock Mackie      Added voxel-by-voxel density variation. 
c   2   Nov. 1,  1986  Rock Mackie      Added diverging fluence calculation
c                                       and block calculations. 
c   3   May  9,  1987  Rock Mackie      Added variable voxel dimensions
c   4   May 16,  1987  Rock Mackie      Export grade documentation. 
c   5   Jan 5,   1988  Rock Mackie      Output unformatted 3-D dose array
c   6   Jan 6,   1988  Rock Mackie      Took out data entry
c   7   Nov 19,  1989  Rock Mackie      Made default the non-interpolative 
c                                       lookup table instead of older 
c                                       interpolation in sphere_convolve
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

        implicit none

        real thickness,ssd,lengthx,lengthy,lengthz                             
        real xmin,xmax,ymin,ymax,incfluence,mu
        real fluence(-63:63,-63:63,1:127),density(-63:63,-63:63,1:127)
        real accessory(-450:450,-450:450)
        real maxdepth,maxlength
        real value_out(-63:63,-63:63,1:127)
        real xminroi,xmaxroi,yminroi,ymaxroi,zminroi,zmaxroi,yplane
        real max,endtime,delta
        real homdens,hetdens
        real xoffset,zoffset,breast_radius
        real tray_factor
        real y_dist,dmax_depth,beam_energy,ray
        real mvalue,bvalue
        real posplane
                               
        integer i,j,k,depthnum,midi
        integer imin,imax,jmin,jmax
        integer mini,maxi,minj,maxj,mink,maxk,jplane
        integer dmaxi,dmaxj,dmaxk
        integer m,minbndi,maxbndi
        integer includ,exclud
        integer hetstate,hetstart,hethick
        integer wedge_angle,iblockmin,iblockmax,jblockmin,jblockmax
        integer planej

        character*1 type_conv,junk,beamtype
        character*3 file_unformat,block,writedose
        character*4 choose
        character*8 now
        character*9 today
        character*40 namefile
        character*80 filename1,filename2,filename3
        character*70 description
        
        common /fluence_common/thickness,xmin,xmax,ymin,ymax,
     1          incfluence,mu,ray,beamtype

        common /all_common/lengthx,lengthy,lengthz,depthnum,imin,imax,
     1          jmin,jmax,fluence,ssd,beam_energy

        common /convolve_common/mini,maxi,mink,maxk,minj,maxj,
     1          value_out,dmaxi,dmaxj,dmaxk,max,endtime
        
        common /enter/filename1,choose,includ,exclud

        common /density/density

        common /accessory_common/accessory 

        common /correction/bvalue,mvalue
                

c Get the following data describing the phantom and the beam geometry.
        
        call convolve_input_data(namefile,      !name of formatted report file
     1                       today,          !today's date
     1                       now,            !time of day
     1                       beam_energy,    !the average primary energy.
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
     2                       ray)            !number of rays per voxel
                                
c Set up the density matrix.

        call density_get(density,lengthx,lengthy,lengthz)

c Set up the beam modifier array.
             
        call modify_calc(accessory,lengthx,lengthy,lengthz,
     1                mu,wedge_angle,tray_factor,ssd, ray)
                                         
c Set up geometry for the fluence calculation.

        depthnum=nint(thickness/lengthz)       !the number of depth voxels.

        print*,'PRIMARY PHOTON RELATIVE FLUENCE BEING CALCULATED'
        print*,' '

        call div_fluence_calc           !calculate the fluence distribution.
             
        print*,'....FLUENCE CALCULATION FINISHED'
        print*,' '                                                   

c Set up geometry for the convolution calculation.

        mini=nint(xminroi/lengthx)     !these values are the numbers
        maxi=nint(xmaxroi/lengthx)     !describing the region of
        mink=nint(zminroi/lengthz)+1   !interest boundaries.
        maxk=nint(zmaxroi/lengthz)
        minj=nint(yminroi/lengthy)     
        maxj=nint(ymaxroi/lengthy)     

c Do the convolution.

        print*,'CONVOLUTION CALCULATION'
        print*,' ' 

c To get the older but slower convolution routine old_sphere_convolve
c that is marginally more accurate in the buildup region
c change the following lines and recompile the code.
                                                    
c       call old_sphere_convolve                !convolve energy.
        call new_sphere_convolve()              !convolve energy.
                              
c        delta=secnds(endtime)/60.0
        delta=10.0

        print*,'......CONVOLUTION FINISHED'
        print*,' ' 
        print*,'Convolution took ...',delta,'  min.'
        print*,' '

        dmax_depth=(float(dmaxk)-0.5)*lengthz 

        print*,' '
c print*,'Do you want the dose distribution written out?'
c print*,'Enter "yes" or "no".'
c accept 7500,writedose
        writedose = 'yes'

        write(10,*)'Is the dose distr. written out ? ...',
     1           writedose

        if (writedose .eq. 'yes' .or. writedose .eq. 'YES') then
        
         print*,' '
c  print*,'Enter the position (cm) of the plane to view the'
c  print*,'distribution. The central axis is at 0.0 cm.'
c  accept*,posplane
         posplane = 0

         write(10,*)'The plane the distribution is ...',posplane

         planej=nint(posplane/lengthy) !determine plane voxel #

c          call date(today)
c          call time(now)           

          open(unit=1,
     1     file='./../code/cono/format_dose.dat',
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
     1                   value_out,          !output
     1                   filename3)          !name of file are writting
                                                                     
         close(1)
        end if

7500    format(a3)
        stop
        end
