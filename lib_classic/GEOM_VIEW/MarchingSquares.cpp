#include <Volumep.h>
#include <Polygon.h>

double calcPoint(double bottom, double top, double c, double tempstep)
{             
	return ((c-bottom)/(top-bottom))*tempstep; 
}


void ComputeIsocurves(CVolume<double> *pVolume, double c, CPolygon *pCurves)
{
	double topleft = 0;
	double topright = 0;
	double bottomleft = 0;
	double bottomright = 0;
	
	double **ppPixels = pVolume->GetVoxels()[0];

	for(int myycounter = 0; myycounter < pVolume->GetHeight(); myycounter++)
	{          
		for(int myxcounter = 0; myycounter < pVolume->GetHeight(); myycounter++)
		{           
			if(myxcounter == 0)
			{
				topleft = ppPixels[myycounter+1][myxcounter];
				topright = ppPixels[myycounter+1][myxcounter+1];
				bottomleft = ppPixels[myycounter][myxcounter];
				bottomright = ppPixels[myycounter][myxcounter+1];
			}
			else
			{
				topleft = topright;
				topright = ppPixels[myycounter+1][myxcounter+1];
				bottomleft = bottomright;
				bottomright = ppPixels[myycounter][myxcounter+1]; 
			}         
			
			//case 1.1
			if((topleft<c && topright<c && bottomleft>c && bottomright>c) 
				|| (topleft>c && topright>c && bottomleft<c && bottomright<c))
			{
				// points are divided down some line in the middle
				double left = calcPoint(bottomleft, topleft, c, stepj);
				double right = calcPoint(bottomright, topright, c, stepj);
				
				pCurves->AddVertex(myxcounter, myycounter + left);
				pCurves->AddVertex(myxcounter+1, myycounter + right);
				linevector.add(new Point3f((float)i,(float)(j+left),(float)(k)));
				linevector.add(new Point3f((float)(i+stepi),(float)(j+right), (float)(k)));
			} 
			//case 1.2 
			else if((topleft<c && topright>c && bottomleft<c && bottomright>c)
				|| (topleft>c && topright<c && bottomleft>c && bottomright<c))
			{
				// points are divided down some line in the middle
				double bottom = calcPoint(bottomleft, bottomright, c, stepi);    
				double top = calcPoint(topleft, topright, c, stepi);
				
				linevector.add(new Point3f((float)(i+bottom),(float)(j),(float)(k)));
				linevector.add(new Point3f((float)(i+top),(float)(j+stepj), (float)(k)));
			} 
			
			//case 2              
			else if((topleft<c && topright<c && bottomleft<c && bottomright<c)
				|| (topleft>c && topright>c && bottomleft>c && bottomright>c)) 
			{
				// all points below or above the line, so dont do anything
			}  
			//case 3.1              
			else if((topleft>c && topright>c && bottomleft>c && bottomright<c)
				||(topleft<c && topright<c && bottomleft<c && bottomright>c))
			{
				// three points above or below the line and the other the opposite
				double right = calcPoint(bottomright, topright, c, stepj);
				double bottom = calcPoint(bottomleft, bottomright, c, stepi);
				
				linevector.add(new Point3f((float)(i+bottom),(float)j,(float)(k)));
				linevector.add(new Point3f((float)(i+stepi),(float)(j+right), (float)(k)));
			}  
			//case 3.2              
			else if((topleft>c && topright>c && bottomleft<c && bottomright>c)
				||(topleft<c && topright<c && bottomleft>c && bottomright<c))
			{
				// three points above or below the line and the other the opposite
				double left = calcPoint(bottomleft, topleft, c, stepj);
				double bottom = calcPoint(bottomleft, bottomright, c, stepi);
				
				linevector.add(new Point3f((float)(i+bottom),(float)j,(float)(k)));
				linevector.add(new Point3f((float)(i),(float)(j+left), (float)(k)));
			}  
			//case 3.3             
			else if((topleft<c && topright>c && bottomleft>c && bottomright>c)
				||(topleft>c && topright<c && bottomleft<c && bottomright<c))
			{
				// three points above or below the line and the other the opposite
				double left = calcPoint(bottomleft, topleft, c, stepj);
				double top = calcPoint(topleft, topright, c, stepi);
				
				linevector.add(new Point3f((float)(i),(float)(j+left),(float)(k)));
				linevector.add(new Point3f((float)(i+top),(float)(j+stepj), (float)(k)));
			}  
			//case 3.4              
			else if((topleft>c && topright<c && bottomleft>c && bottomright>c)
				||(topleft<c && topright>c && bottomleft<c && bottomright<c))
			{
				// three points above or below the line and the other the opposite
				double top = calcPoint(topleft, topright, c, stepi);
				double right = calcPoint(bottomright, topright, c, stepj);
				
				linevector.add(new Point3f((float)(i+top),(float)(j+stepj),(float)(k)));
				linevector.add(new Point3f((float)(i+stepi),(float)(j+right), (float)(k)));
			}  
			
			//case 4.1              
			else if(topleft>c && topright<c && bottomleft<c && bottomright>c)
			{
				//Ambigous case where the corners are diametrically opposed
				//Test in the middle and decide which way the lines go..
				double middle = SFS.val(new Vector3d((double)(i+(stepi/2)), (double)(j+(stepj/2)), 0.0));
				
				double top = calcPoint(topleft, topright, c, stepi);
				double bottom = calcPoint(bottomleft, bottomright, c, stepi);
				double left = calcPoint(bottomleft, topleft, c, stepj);
				double right = calcPoint(bottomright, topright, c, stepj);
				
				if(middle < c)
				{
					linevector.add(new Point3f((float)(i),(float)(j+left),(float)(k)));
					linevector.add(new Point3f((float)(i+top),(float)(j+stepj), (float)(k)));
					
					linevector.add(new Point3f((float)(i+bottom),(float)(j),(float)(k)));
					linevector.add(new Point3f((float)(i+stepi),(float)(j+right), (float)(k)));  
				}
				else if(middle > c)
				{
					linevector.add(new Point3f((float)(i),(float)(j+left),(float)(k)));
					linevector.add(new Point3f((float)(i+bottom),(float)(j),(float)(k)));
					
					linevector.add(new Point3f((float)(i+top),(float)(j+stepj), (float)(k)));
					linevector.add(new Point3f((float)(i+stepi),(float)(j+right), (float)(k)));
				}
				
			}
			//case 4.2
			else if(topleft<c && topright>c && bottomleft>c && bottomright<c)
			{
				//Ambigous case where the corners are diametrically opposed
				//Test in the middle and decide which way the lines go..
				double middle = SFS.val(new Vector3d((double)(i+(stepi/2)), (double)(j+(stepj/2)), 0.0));
				double top = calcPoint(topleft, topright, c, stepi);
				double bottom = calcPoint(bottomleft, bottomright, c, stepi);
				double left = calcPoint(bottomleft, topleft, c, stepj);
				double right = calcPoint(bottomright, topright, c, stepj);
				
				if(middle<c)
				{
					linevector.add(new Point3f((float)(i),(float)(j+left),(float)(k)));
					linevector.add(new Point3f((float)(i+bottom),(float)(j), (float)(k)));
					
					linevector.add(new Point3f((float)(i+top),(float)(j+stepj),(float)(k)));
					linevector.add(new Point3f((float)(i+stepi),(float)(j+right), (float)(k)));   
				}
				else if(middle>c)
				{
					linevector.add(new Point3f((float)(i),(float)(j+left),(float)(k)));
					linevector.add(new Point3f((float)(i+top),(float)(j+stepj),(float)(k)));
					
					linevector.add(new Point3f((float)(i+bottom),(float)(j), (float)(k)));
					linevector.add(new Point3f((float)(i+stepi),(float)(j+right), (float)(k)));  
				}
				
			}
			
			myxcounter++;
    }//close i-loop
	
	i = imin;
	myycounter++;
	
 }//close j-loop
 
 i = imin;
 j = jmin;
 myzcounter++;
 
} // close k-loop


