        subroutine ray_trace_set_up(numphi,     !number of zenith angles
     1                           numthet,    !number of azimuthal angles
     1                           numstep,    !number of voxels passed thru
     1                           ang,        !zenith angle list           
     1                           lengthx,    !dim. of voxel in x-dir
     1                           lengthy,    !dim. of voxel in y-dir
     1                           lengthz,    !dim. of voxel in z-dir
     1                           r,          !list of lengths thru voxels
     1                           delta_i,    !list of voxel # in x-dir
     1                           delta_j,    !list of voxel # in y-dir
     1                           delta_k)    !list of voxel # in z-dir
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c       Copyright (c) 1988, Rock Mackie
c                           University of Wisconsin
c                           Madison, WI
c
c Key Words: ray-trace, boundary crossing
c                                        
c Function: Sets up arrays used for ray-tracing. The arrays contain the
c           voxel locations and the ray-trace length through the voxel.
c           The ray-tracing is done into a set of zenith directions and
c           evenly distributed about all azimuthal directions (0 to 2 pi).
c           it could be used to "view" through a conical distribution
c           of diverging rays.
c                                                            
c Filename: ray_trace_set_up
c Comment for Programmers: Called from NEW_SPHERE_CONVOLVE. The maximum
c                          length of the location and length lists are
c                          presently set to 64 steps and 48 zenith and
c                          48 azimuthal angles (i.e. 48x48 vectors can
c                          specified each with 64 steps. The zenith angle
c                          is the angle of the vector with respect to 
c                          the z-direction and the azimuthal angle is 
c                          the angle between the projection of the vector
c                          onto the x-y plane and the x-direction. These
c                          values should be set in a parameter statement.
c Revisions:                                     
c  Rev.                 Name of 
c   #       Date        Revisor         Revision(s) Made
c   0   Feb. 4, 1988    Rock Mackie     Program written.
c   1   June. 14, 1988  Rock Mackie     More documentation 
c   2   Jan. 27, 1989   Rock Mackie     Still more documentation
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

        implicit none
        
        integer phi,numphi,thet,numthet,numstep,delta_i_x(64)
        integer delta_j_x(64),delta_k_x(64)
        integer delta_i_y(64),delta_j_y(64)       
        integer delta_k_y(64),delta_i_z(64)       
        integer delta_j_z(64),delta_k_z(64)
        integer delta_i(64,48,48),delta_j(64,48,48),delta_k(64,48,48)
        integer i,j,k,n                    
                
        real pi,ang_inc,sphi,cphi,ang(48),sthet,cthet,lengthx
        real lengthy,lengthz,rx(64),ry(64),rz(64)
        real r(0:64,48,48),last_radius
        
        pi=3.141593
        ang_inc=2.0*pi/float(numthet) !the azimuthal angle increment 
                                      !is calculated
        do phi=1,numphi              !loop thru all zenith angles
        
         sphi=sin(ang(phi))          !trig. for zenith angles
         cphi=cos(ang(phi))

         do thet=1,numthet                  !loop thru all azimuthal angles
        
          sthet=sin(float(thet)*ang_inc)    !trig. for azimuthal angles
          cthet=cos(float(thet)*ang_inc)

c MAKE_VECTOR is called for each direction. It returns the distance from
c the intersection of a plane defined by a coordinate value and the ray along
c each direction.                                

c Call for the y-z plane crossing list. Plane defined by the x-coordinate.
        
          call make_vector(numstep,              
     1                  sphi*cthet/lengthx,   !x-dir direction cosine
     1                  sphi*sthet/lengthy,   !y-dir direction cosine 
     1                  cphi/lengthz,         !z-dir direction cosine    
     1                  rx,                   !list of dist. to x-bound  
     1                  delta_i_x,            !x-dir voxel location      
     1                  delta_j_x,            !y-dir voxel location      
     1                  delta_k_x)            !z-dir voxel location      
                                                                      
c Call for the x-z plane crossing list. Plane defined by the y-coordinate.
                                                                         
          call make_vector(numstep,              
     1                  sphi*sthet/lengthy,   !y-dir direction cosine    
     1                  sphi*cthet/lengthx,   !x-dir direction cosine    
     1                  cphi/lengthz,         !z-dir direction cosine    
     1                  ry,                   !list of dist. to y-bound  
     1                  delta_j_y,            !y-dir voxel location      
     1                  delta_i_y,            !x-dir voxel location      
     1                  delta_k_y)            !z-dir voxel location      
        
c Call for the x-y plane crossing list. Plane defined by the z-coordinate.
                                                                 
          call make_vector(numstep,              
     1                  cphi/lengthz,         !z-dir direction cosine    
     1                  sphi*sthet/lengthy,   !y-dir direction cosine    
     1                  sphi*cthet/lengthx,   !x-dir direction cosine    
     1                  rz,                   !list of dist. to z-bound  
     1                  delta_k_z,            !z-dir voxel location      
     1                  delta_j_z,            !y-dir voxel location      
     1                  delta_i_z)            !x-dir voxel location      
                                                  
          i=1
          j=1
          k=1
          r(0,phi,thet)=0.0             !radius at origin is 0
          last_radius=0.0

c The following sorts through the distance vectors, rx,ry,rz
c to find the next smallest value. This will be the next plane crossed.
c A merged vector is created that lists the location of the voxel crossed
c and the length thru it in the order of crossings.
                                                  
          do n=1,numstep
                                                               !done if plane
           if (rx(i) .le. ry(j) .and. rx(i) .le. rz(k)) then   !defined by the
                                                               !x-coord crossed
            r(n,phi,thet)=rx(i)-last_radius       !length thru voxel
            delta_i(n,phi,thet)=delta_i_x(i)      !location of voxel in x-dir.
            delta_j(n,phi,thet)=delta_j_x(i)      !location of voxel in y-dir.
            delta_k(n,phi,thet)=delta_k_x(i)      !location of voxel in z-dir.
            last_radius=rx(i)                     !radius gone at this point
            i=i+1                                 !decrement x-dir counter

                                                               !done if plane
      else if (ry(j) .le. rx(i) .and. ry(j) .le. rz(k)) then   !defined by the
                                                               !y-coord crossed
            r(n,phi,thet)=ry(j)-last_radius       !length thru voxel
            delta_i(n,phi,thet)=delta_i_y(j)      !location of voxel in x-dir.
            delta_j(n,phi,thet)=delta_j_y(j)      !location of voxel in y-dir.
            delta_k(n,phi,thet)=delta_k_y(j)      !location of voxel in z-dir.
            last_radius=ry(j)                     !radius gone at this point
            j=j+1                                 !decrement y-dir counter

                                                               !done if plane
      else if (rz(k) .le. rx(i) .and. rz(k) .le. ry(j)) then   !defined by the
                                                               !z-coord crossed
            r(n,phi,thet)=rz(k)-last_radius       !length thru voxel
            delta_i(n,phi,thet)=delta_i_z(k)      !location of voxel in x-dir.
            delta_j(n,phi,thet)=delta_j_z(k)      !location of voxel in y-dir.
            delta_k(n,phi,thet)=delta_k_z(k)      !location of voxel in z-dir.
            last_radius=rz(k)                     !radius gone at this point
            k=k+1                                 !decrement y-dir counter
                                                              
           end if
        
          end do

         end do
        
        end do
        
        return
        
        end
