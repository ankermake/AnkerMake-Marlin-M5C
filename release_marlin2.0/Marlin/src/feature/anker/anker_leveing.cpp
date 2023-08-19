/*
 * @Author       : harley
 * @Date         : 2022-04-31 20:35:23
 * @LastEditors  : harley
 * @LastEditTime : 
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"


#if ENABLED(AUTO_BED_LEVELING_BILINEAR)&&ENABLED(ANKER_ROTATION)
 #include "anker_leveing.h"
 #include "../../feature/bedlevel/bedlevel.h"
    Anker_Rotation anker_rotation;

    void Anker_Rotation:: convert_rotation_value(void)
    {
         int8_t j;
            //  for(j=0;j<GRID_MAX_POINTS_Y;j++)
            //  {
            //    z_values[0][j]+=0.12;//anker_rotation.rotation_value;
            //  }
            //  for(j=0;j<GRID_MAX_POINTS_Y;j++)
            //  {
            //    z_values[1][j]+=0.08;//anker_rotation.rotation_value/2;
            //  }
            //  for(j=0;j<GRID_MAX_POINTS_Y;j++)
            //  {
            //    z_values[2][j]+=0.04;//anker_rotation.rotation_value/4;
            //  }
            //  for(j=0;j<GRID_MAX_POINTS_Y;j++)
            //  {
            //    z_values[4][j]-=0.04;//anker_rotation.rotation_value/4;
            //  }
            //  for(j=0;j<GRID_MAX_POINTS_Y;j++)
            //  {
            //    z_values[5][j]-=0.08;//anker_rotation.rotation_value/2;
            //  }
            //  for(j=0;j<GRID_MAX_POINTS_Y;j++)
            //  {
            //    z_values[6][j]-=0.12;//anker_rotation.rotation_value;
            //  }

             for(j=0;j<GRID_MAX_POINTS_Y;j++)
             {
               z_values[0][j]+=0.08;//anker_rotation.rotation_value;
             }
             for(j=0;j<GRID_MAX_POINTS_Y;j++)
             {
               z_values[1][j]+=0.04;//anker_rotation.rotation_value/2;
             }
             for(j=0;j<GRID_MAX_POINTS_Y;j++)
             {
               z_values[3][j]-=0.04;//anker_rotation.rotation_value/4;
             }
             for(j=0;j<GRID_MAX_POINTS_Y;j++)
             {
               z_values[4][j]-=0.08;//anker_rotation.rotation_value/4;
             }
    }
#endif

#if ENABLED(ANKER_LEVEING)
   Anker_Leveing anker_leveing;

    xy_pos_t Anker_Leveing::xy[GRID_MAX_POINTS_X];

       void Anker_Leveing :: test(uint8_t select)
       {
          anker_leveing.xy[0].x=PROBING_MARGIN;
          anker_leveing.xy[0].y=Y_CENTER;

             switch (select)
             {
              case 1:
                  
                break;
              case 2:

                break;
              case 3:

                break;
              case 4:

                break;
              case 5:

                break;
              
              default:
                break;
             }
       }

#endif

