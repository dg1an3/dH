        subroutine energy_lookup(rad_bound,
     1                        numrad,
     1                        phi,
     1                        inc_energy,
     1                        cum_energy)
                                            
        implicit none
                     
        integer phi,i,r,rad_numb,numrad
        
        real rad_bound(0:48),rad_dist,last_rad
        real inc_energy(48,0:48),cum_energy(48,0:600),tot_energy

        rad_numb=1
        last_rad=0.0
        rad_dist=0.0
        cum_energy(phi,0)=0.0
                                            
        do i=1,599
        
         rad_dist=i*0.1
                       
         do r=rad_numb-1,numrad      
                                    
          if (rad_bound(r) .gt. rad_dist) go to 100
                               
         end do
              
100      rad_numb=r

c New subroutine interpolates the kernel values to get the 
c energy absorbed.
               
         call interp_energy(last_rad,           !inner boundary
     1                   rad_dist,           !outer boundary
     1                   rad_numb,           !radial label of inner voxel
     1                   phi,                !angular label of voxel
     1                   rad_bound,          !radius of voxel boundaries
     1                   inc_energy,         !array of incremental energy
     1                   tot_energy)         !incr. energy. at bound.
                                                                               
         last_rad=rad_dist 
        
         cum_energy(phi,i)=tot_energy
                       
        end do

        return
        
        end
              
