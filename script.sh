#! /bin/bash -e



                                                                                                                       
if [ $# -ne 1 ]; then                                                                                                  
        echo "usage : $0 event";                                                                                       
        exit 1                                                                                                         
fi                                                                                                                     
                                                                                                                       
rep="/var/www/html/ReconCam/"                                                                                          
cd $rep                                                                                                                
                                                                                                                       
mkdir -p alert                                                                                                         
touch alert/$1                                                                                                         
                                                                                                                       
sleep 18                                                                                                               
                                                                                                                       
rm alert/$1                                                                                                            
                                                                                                                       
                                                                                                                       
# if [ -z "$(ls alert)" ]; then                                                                                        
    # rm alert.jpg                                                                                                     
# fi                                                                                                                   
find alert/ -maxdepth 0 -empty -exec rm alert.jpg \;                                                                   
                                                                                                                       
