
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <sys/stat.h>


#include "const.h"
#include "2d_array.h"
#include "utilities.h"
#include "input.h"
#include "scene_based_lst.h"


#ifndef max
    #define max(a,b) (((a) (b)) ? (a) : (b))
#endif


#ifndef min
    #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

/******************************************************************************
MODULE:  convert_geopotential_geometric

PURPOSE: Convert array of geopotential heights to array of geometric heights
         given latitude

RETURN: SUCCESS
        FAILURE

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
9/22/2014   Song Guo         Original Development
******************************************************************************/
int convert_geopotential_geometric
(
    int num_points,
    float *lat,
    float **geo_potential,
    float **geo_metric
)
{
    float *radlat;
    int i, j;
    float g_0 = 9.80665;
    float r_max = 6378.137;
    float r_min = 6356.752;
    float *radius;
    float *gravity_ratio;

    /* Allocate memeory */
    radlat = (float *) malloc (num_points * sizeof (float));
    if (radlat == NULL)
    {
        ERROR_MESSAGE ("Allocating radlat memory",
                       "convert_geopotential_geometric");
    }

    radius = (float *) malloc (num_points * sizeof (float));
    if (radius == NULL)
    {
        ERROR_MESSAGE ("Allocating radius memory",
                       "convert_geopotential_geometric");
    }

    gravity_ratio = (float *) malloc (num_points * sizeof (float));
    if (gravity_ratio == NULL)
    {
        ERROR_MESSAGE ("Allocating gravity_ratio memory",
                       "convert_geopotential_geometric");
    }

    for (i = 0; i < num_points; i++)
    {
        radlat[i] = (lat[i] * PI) / 180.0;

        /* define variable based on latitude */
        radius[i] = 1000.0
                    * (sqrt (1.0 / ((cos (radlat[i]) * cos (radlat[i]))
                                    / (r_max * r_max)
                                    + ((sin (radlat[i]) * sin (radlat[i]))
                                       / (r_min * r_min)))));
        gravity_ratio[i] = (9.80616
                            * (1 - 0.002637 * cos (2.0 * radlat[i])
                               + 0.0000059 * (cos (2.0 * radlat[i])
                                              * cos (2.0 * radlat[i]))))
                           / g_0;
    }

    for (i = 0; i < P_LAYER; i++)
    {
        for (j = 0; j < num_points; j++)
        {
            geo_metric[i][j] = (geo_potential[i][j] * radius[j])
                / (1000.0 * (gravity_ratio[j] * radius[j]
                             - geo_potential[i][j]));
        }
    }

    /* free memory */
    free (radlat);
    free (radius);
    free (gravity_ratio);

    return SUCCESS;
}


/******************************************************************************
MODULE:  convert_sh_rh

PURPOSE: Given array of specific humidities, temperature, and pressure,
         generate array of relative humidities

RETURN: SUCCESS
        FAILURE

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
9/22/2014   Song Guo         Original Development
******************************************************************************/
int convert_sh_rh
(
    int num_points,
    float *lat,
    float **spec_hum,
    float **temp_k,
    float **pressure,
    float **rh
)
{
    int i, j;
    float mh20 = 18.01534;
    float mdry = 28.9644;

    float a0w = 6.107799961;
    float a1w = 4.436518521e-1;
    float a2w = 1.428945805e-2;
    float a3w = 2.650648471e-4;
    float a4w = 3.0312403963e-6;
    float a5w = 2.034080948e-8;
    float a6w = 6.136820929e-11;
    float **temp_c;
    float **ewater;
    float **e2;
    float **goff;
    float **ph20;
    int status;

    /* Allocate memory */
    temp_c =
        (float **) allocate_2d_array (P_LAYER, num_points, sizeof (float));
    if (temp_c == NULL)
    {
        ERROR_MESSAGE ("Allocating temp_c memory", "first_files");
    }

    ewater =
        (float **) allocate_2d_array (P_LAYER, num_points, sizeof (float));
    if (ewater == NULL)
    {
        ERROR_MESSAGE ("Allocating ewater memory", "first_files");
    }

    e2 = (float **) allocate_2d_array (P_LAYER, num_points, sizeof (float));
    if (e2 == NULL)
    {
        ERROR_MESSAGE ("Allocating e2 memory", "first_files");
    }

    goff = (float **) allocate_2d_array (P_LAYER, num_points, sizeof (float));
    if (goff == NULL)
    {
        ERROR_MESSAGE ("Allocating goff memory", "first_files");
    }

    ph20 = (float **) allocate_2d_array (P_LAYER, num_points, sizeof (float));
    if (ph20 == NULL)
    {
        ERROR_MESSAGE ("Allocating  memory", "first_files");
    }

    for (i = 0; i < P_LAYER; i++)
    {
        for (j = 0; j < num_points; j++)
        {
            /* Convert temperature to C */
            temp_c[i][j] = temp_k[i][j] - 273.15;
            /* calculate vapor pressure at given temperature */
            ewater[i][j] = a0w + temp_c[i][j] * (a1w + temp_c[i][j] * (a2w + temp_c[i][j] * (a3w + temp_c[i][j] * (a4w + temp_c[i][j] * (a5w + temp_c[i][j] * (a6w * temp_c[i][j]))))));        /* hpa */

            e2[i][j] = exp (-0.58002206e4 / temp_k[i][j] + 0.13914993 - 0.48640239e-1 * temp_k[i][j] + 0.41764768e-4 * pow (temp_k[i][j], 2.0) - 0.14452093e-7 * pow (temp_k[i][j], 3.0) + 0.65459673 * log (temp_k[i][j]));    /* Pa */

            goff[i][j] = -7.90298 * (373.16 / temp_k[i][j] - 1) + 5.02808 * log10 (373.16 / temp_k[i][j]) - 1.3816e-7 * pow (10.0, (11.344 * (1.0 - (temp_k[i][j] / 373.16))) - 1.0) + 8.1328e-3 * pow (10.0, (-3.49149 * (373.16 / temp_k[i][j] - 1.0)) - 1.0) + log10 (1013.246);     /* hPa */

            ph20[i][j] = (spec_hum[i][j] * pressure[i][j] * mdry)
                / (mh20 - spec_hum[i][j] * mh20 + spec_hum[i][j] * mdry);

            rh[i][j] = (ph20[i][j] / pow (10.0, goff[i][j])) * 100.0;
        }
    }

    /* Free allocated memory */
    status = free_2d_array ((void **) temp_c);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: temp_c\n", "first_files");
    }

    status = free_2d_array ((void **) ewater);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: ewater\n", "first_files");
    }

    status = free_2d_array ((void **) e2);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: e2\n", "first_files");
    }

    status = free_2d_array ((void **) goff);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: goff\n", "first_files");
    }

    status = free_2d_array ((void **) ph20);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: ph20\n", "first_files");
    }

    return SUCCESS;
}


/******************************************************************************
MODULE:  build_modtran_input

PURPOSE: Creates directories and writes tape5 file, caseList, and commandList

RETURN: SUCCESS
        FAILURE

HISTORY:
Date        Programmer       Reason
--------    ---------------  -------------------------------------
9/21/2014   Song Guo         Original Development
******************************************************************************/
int build_modtran_input
(
    Input_t *input,  /*I: input structure */
    int *num_points, /*O: number of NARR points */
    bool verbose     /*I: value to indicate if intermediate messages
                          be printed */
)
{
    int **eye;
    int **jay;
    float **lat;
    float **lon;
    float **hgt1;
    float **shum1;
    float **tmp1;
    float **hgt2;
    float **shum2;
    float **tmp2;
    float *narr_lat;
    float *narr_lon;
    float **narr_hgt1;
    float **narr_shum1;
    float **narr_tmp1;
    float **narr_hgt2;
    float **narr_shum2;
    float **narr_tmp2;
    float **narr_height;
    float **narr_height1;
    float **narr_height2;
    float **narr_rh;
    float **narr_rh1;
    float **narr_rh2;
    float **narr_tmp;
    int i, j, k;
    int p[P_LAYER] = { 1000, 975, 950, 925, 900,
        875, 850, 825, 800, 775,
        750, 725, 700, 650, 600,
        550, 500, 450, 400, 350,
        300, 275, 250, 225, 200,
        175, 150, 125, 100
    };
    char full_path[MAX_STR_LEN];
//    int landsat_hemi;
    float narr_ul_lat;
    float narr_ul_lon;
    float narr_lr_lat;
    float narr_lr_lon;
    int in_counter = 0;
    int max_eye;
    int min_eye;
    int max_jay;
    int min_jay;
    int num_eyes;
    int num_jays;
    float **pressure;
    int rem1;
    int rem2;
    float hour1;
    float hour2;
    float time;
    FILE *fd;
    float *stan_height;
    float *stan_pre;
    float *stan_temp;
    float *stan_rh;
    float *temp_height;
    float *temp_pressure;
    float *temp_temp;
    float *temp_rh;
    float gndalt[NUM_ELEVATIONS] = { 0.0, 0.6, 1.1, 1.6, 2.1, 2.6,
        3.1, 3.6, 4.05
    };
    int num_cases;
    char command[MAX_STR_LEN];
    char current_gdalt[MAX_STR_LEN];
    char current_temp[MAX_STR_LEN];
    char current_alb[MAX_STR_LEN];
    char current_point[MAX_STR_LEN];
    char temp_out[MAX_STR_LEN];
    char curr_path[MAX_STR_LEN];
    int index_below = 0;
    int index_above = NUM_ELEVATIONS;
    float new_height;
    float new_pressure;
    float new_temp;
    float new_rh;
    int index, index2;
    int *counter;
    float tmp[3] = { 273.0, 310.0, 0.0 };
    float alb[3] = { 0.0, 0.0, 0.1 };
    char *path = NULL;
    char *modtran_path = NULL;
    char *modtran_data_dir = NULL;
    struct stat s;
    int status;
    int temp_int1, temp_int2;
    int case_counter;
    char **case_list;
    char **command_list;

    /* Dynamic allocate the 2d memory */
    eye = (int **) allocate_2d_array (NARR_ROW, NARR_COL, sizeof (int));
    if (eye == NULL)
    {
        ERROR_MESSAGE ("Allocating eye memory", "first_files");
    }

    jay = (int **) allocate_2d_array (NARR_ROW, NARR_COL, sizeof (int));
    if (jay == NULL)
    {
        ERROR_MESSAGE ("Allocating jay memory", "first_files");
    }

    lat = (float **) allocate_2d_array (NARR_ROW, NARR_COL, sizeof (float));
    if (lat == NULL)
    {
        ERROR_MESSAGE ("Allocating lat memory", "first_files");
    }

    lon = (float **) allocate_2d_array (NARR_ROW, NARR_COL, sizeof (float));
    if (lon == NULL)
    {
        ERROR_MESSAGE ("Allocating lon memory", "first_files");
    }

    path = getenv ("LST_DATA_DIR");
    if (path == NULL)
    {
        ERROR_MESSAGE ("LST_DATA_DIR environment variable is not set",
                       "first_files");
    }

    sprintf (full_path, "%s/%s", path, "narr_coordinates.txt");
    fd = fopen (full_path, "r");
    if (fd == NULL)
    {
        ERROR_MESSAGE ("Can't open narr_coordinates.txt file", "first_files");
    }

    for (j = 0; j < NARR_COL; j++)
    {
        for (i = 0; i < NARR_ROW; i++)
        {
            if (fscanf (fd, "%d %d %f %f", &eye[i][j], &jay[i][j], &lat[i][j],
                        &lon[i][j]) == EOF)
            {
                ERROR_MESSAGE ("End of file (EOF) is met before"
                               " NARR_ROW * NARR_COL lines", "first_file");
            }
            if ((lon[i][j] - 180.0) > MINSIGMA)
                lon[i][j] = 360.0 - lon[i][j];
            else
                lon[i][j] = -lon[i][j];
        }
    }
    fclose (fd);

    /* Dynamic allocate the 2d memory */
    hgt1 = (float **) allocate_2d_array (P_LAYER, NARR_ROW * NARR_COL,
                                         sizeof (float));
    if (hgt1 == NULL)
    {
        ERROR_MESSAGE ("Allocating hgt_1 memory", "first_files");
    }

    shum1 = (float **) allocate_2d_array (P_LAYER, NARR_ROW * NARR_COL,
                                          sizeof (float));
    if (shum1 == NULL)
    {
        ERROR_MESSAGE ("Allocating shum_1 memory", "first_files");
    }

    tmp1 = (float **) allocate_2d_array (P_LAYER, NARR_ROW * NARR_COL,
                                         sizeof (float));
    if (tmp1 == NULL)
    {
        ERROR_MESSAGE ("Allocating tmp_1 memory", "first_files");
    }

    /* Read in NARR height for time before landsat acqusition */
    for (i = 0; i < P_LAYER; i++)
    {
        sprintf (full_path, "%s/%d%s", "HGT_1/", p[i], ".txt");
        fd = fopen (full_path, "r");
        if (fd == NULL)
        {
            ERROR_MESSAGE ("Can't HGT_1 txt file", "first_files");
        }

        fscanf (fd, "%d %d", &temp_int1, &temp_int2);
        for (j = 0; j < NARR_ROW * NARR_COL; j++)
        {
            if (fscanf (fd, "%f", &hgt1[i][j]) == EOF)
            {
                ERROR_MESSAGE ("End of file (EOF) is met before "
                               "NARR_ROW * NARR_COL lines", "first_files");
            }
        }
        fclose (fd);
    }

    /* Read in NARR specific humidity for time before landsat acqusition */
    for (i = 0; i < P_LAYER; i++)
    {
        sprintf (full_path, "%s/%d%s", "SHUM_1/", p[i], ".txt");
        fd = fopen (full_path, "r");
        if (fd == NULL)
        {
            ERROR_MESSAGE ("Can't open SHUM_1 file", "first_files");
        }

        fscanf (fd, "%d %d", &temp_int1, &temp_int2);
        for (j = 0; j < NARR_ROW * NARR_COL; j++)
        {
            if (fscanf (fd, "%f", &shum1[i][j]) == EOF)
            {
                ERROR_MESSAGE ("End of file (EOF) is met before "
                               "NARR_ROW * NARR_COL lines", "first_files");
            }
        }
        fclose (fd);
    }

    /* Read in NARR temperature for time before landsat acqusition */
    for (i = 0; i < P_LAYER; i++)
    {
        sprintf (full_path, "%s/%d%s", "TMP_1/", p[i], ".txt");
        fd = fopen (full_path, "r");
        if (fd == NULL)
        {
            ERROR_MESSAGE ("Can't open TMP_1 file", "first_files");
        }

        fscanf (fd, "%d %d", &temp_int1, &temp_int2);
        for (j = 0; j < NARR_ROW * NARR_COL; j++)
        {
            if (fscanf (fd, "%f", &tmp1[i][j]) == EOF)
            {
                ERROR_MESSAGE ("End of file (EOF) is met before "
                               "NARR_ROW * NARR_COL lines", "first_files");
            }
        }
        fclose (fd);
    }

    /* Dynamic allocate the 2d memory */
    hgt2 = (float **) allocate_2d_array (P_LAYER, NARR_ROW * NARR_COL,
                                         sizeof (float));
    if (hgt2 == NULL)
    {
        ERROR_MESSAGE ("Allocating hgt_2 memory", "first_files");
    }

    shum2 = (float **) allocate_2d_array (P_LAYER, NARR_ROW * NARR_COL,
                                          sizeof (float));
    if (shum2 == NULL)
    {
        ERROR_MESSAGE ("Allocating shum_2 memory", "first_files");
    }

    tmp2 = (float **) allocate_2d_array (P_LAYER, NARR_ROW * NARR_COL,
                                         sizeof (float));
    if (tmp2 == NULL)
    {
        ERROR_MESSAGE ("Allocating tmp_2 memory", "first_files");
    }

    /* Read in NARR height for time after landsat acqusition */
    for (i = 0; i < P_LAYER; i++)
    {
        sprintf (full_path, "%s/%d%s", "HGT_2/", p[i], ".txt");
        fd = fopen (full_path, "r");
        if (fd == NULL)
        {
            ERROR_MESSAGE ("Can't HGT_2 txt file", "first_files");
        }

        fscanf (fd, "%d %d", &temp_int1, &temp_int2);
        for (j = 0; j < NARR_ROW * NARR_COL; j++)
        {
            if (fscanf (fd, "%f", &hgt2[i][j]) == EOF)
            {
                ERROR_MESSAGE ("End of file (EOF) is met before "
                               "NARR_ROW * NARR_COL lines", "first_files");
            }
        }
        fclose (fd);
    }

    /* Read in NARR specific humidity for time after landsat acqusition */
    for (i = 0; i < P_LAYER; i++)
    {
        sprintf (full_path, "%s/%d%s", "SHUM_2/", p[i], ".txt");
        fd = fopen (full_path, "r");
        if (fd == NULL)
        {
            ERROR_MESSAGE ("Can't open SHUM_2 file", "first_files");
        }

        fscanf (fd, "%d %d", &temp_int1, &temp_int2);
        for (j = 0; j < NARR_ROW * NARR_COL; j++)
        {
            if (fscanf (fd, "%f", &shum2[i][j]) == EOF)
            {
                ERROR_MESSAGE ("End of file (EOF) is met before "
                               "NARR_ROW * NARR_COL lines", "first_files");
            }
        }
        fclose (fd);
    }

    /* Read in NARR temperature for time after landsat acqusition */
    for (i = 0; i < P_LAYER; i++)
    {
        sprintf (full_path, "%s/%d%s", "TMP_2/", p[i], ".txt");
        fd = fopen (full_path, "r");
        if (fd == NULL)
        {
            ERROR_MESSAGE ("Can't open TMP_2 file", "first_files");
        }

        fscanf (fd, "%d %d", &temp_int1, &temp_int2);
        for (j = 0; j < NARR_ROW * NARR_COL; j++)
        {
            if (fscanf (fd, "%f", &tmp2[i][j]) == EOF)
            {
                ERROR_MESSAGE ("End of file (EOF) is met before "
                               "NARR_ROW * NARR_COL lines", "first_files");
            }
        }
        fclose (fd);
    }

    /* determine if landsat is in the northern or southern hemisphere.
       '6' = northern hemisphere, '7' = southern hermisphere. */
//    if (input->meta.ul_geo_corner.lat > MINSIGMA)
//        landsat_hemi = 6;
//    else
//        landsat_hemi = 7; 

    /* expand range to include NARR points outside image for edge pixels */
    narr_ul_lat = input->meta.ul_geo_corner.lat + 0.5;
    narr_ul_lon = input->meta.ul_geo_corner.lon - 0.5;
    narr_lr_lat = input->meta.lr_geo_corner.lat - 0.5;
    narr_lr_lon = input->meta.lr_geo_corner.lon + 0.5;

    /* determine what points in the NARR dataset fall within the Landsat
       image using logical operators lessThanLat and greaterThanLat are values
       where the NARR values are less than or greater than the edges of the
       Landsat corners values respectively pixels that are true in both fall
       within the Landsat scene the same thing is done with longitude values */
    max_eye = 0;
    min_eye = 1000;
    max_jay = 0;
    min_jay = 1000;
    for (i = 0; i < NARR_ROW - 1; i++)
    {
        for (j = 0; j < NARR_COL - 1; j++)
        {
            if ((lat[i][j] - narr_ul_lat) < MINSIGMA
                && (lat[i][j] - narr_lr_lat) > MINSIGMA
                && (lon[i][j] - narr_lr_lon) < MINSIGMA
                && (lon[i][j] - narr_ul_lon) > MINSIGMA)
            {
                max_eye = max (max_eye, eye[i][j]);
                min_eye = min (min_eye, eye[i][j]);
                max_jay = max (max_jay, jay[i][j]);
                min_jay = min (min_jay, jay[i][j]);
                in_counter++;
            }
        }
    }
    max_eye--;
    min_eye--;
    max_jay--;
    min_jay--;
    num_eyes = max_eye - min_eye + 1;
    num_jays = max_jay - min_jay + 1;
    *num_points = num_eyes * num_jays;

    /* Allocate memory for height of NARR points within the rectangular */
    narr_lat = (float *) malloc (*num_points * sizeof (float));
    if (narr_lat == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_lat memory", "first_files");
    }

    narr_lon = (float *) malloc (*num_points * sizeof (float));
    if (narr_lon == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_lon memory", "first_files");
    }

    narr_hgt1 = (float **) allocate_2d_array (P_LAYER, *num_points,
                                              sizeof (float));
    if (narr_hgt1 == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_hgt1 memory", "first_files");
    }

    narr_hgt2 = (float **) allocate_2d_array (P_LAYER, *num_points,
                                              sizeof (float));
    if (narr_hgt2 == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_hgt2 memory", "first_files");
    }

    /* Allocate memory for humidity of NARR points within the rectangular */
    narr_shum1 = (float **) allocate_2d_array (P_LAYER, *num_points,
                                               sizeof (float));
    if (narr_shum1 == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_shum1 memory", "first_files");
    }

    narr_shum2 = (float **) allocate_2d_array (P_LAYER, *num_points,
                                               sizeof (float));
    if (narr_shum2 == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_shum2 memory", "first_files");
    }

    /* Allocate memory for temperature of NARR points within the rectangular */
    narr_tmp1 = (float **) allocate_2d_array (P_LAYER, *num_points,
                                              sizeof (float));
    if (narr_tmp1 == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_tmp1 memory", "first_files");
    }

    narr_tmp2 = (float **) allocate_2d_array (P_LAYER, *num_points,
                                              sizeof (float));
    if (narr_tmp2 == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_tmp2 memory", "first_files");
    }

    /* extract coordinates within the NARR rectangle */
    for (j = min_jay; j <= max_jay; j++)
    {
        for (i = min_eye; i <= max_eye; i++)
        {
            narr_lat[(j - min_jay) * num_eyes + (i - min_eye)] = lat[i][j];
            narr_lon[(j - min_jay) * num_eyes + (i - min_eye)] = lon[i][j];
        }
    }
    for (k = 0; k < P_LAYER; k++)
    {
        for (j = min_jay; j <= max_jay; j++)
        {
            for (i = min_eye; i <= max_eye; i++)
            {
                narr_hgt1[k][(j - min_jay) * num_eyes + (i - min_eye)]
                    = hgt1[k][j * NARR_ROW + i];
                narr_shum1[k][(j - min_jay) * num_eyes + (i - min_eye)]
                    = shum1[k][j * NARR_ROW + i];
                narr_tmp1[k][(j - min_jay) * num_eyes + (i - min_eye)]
                    = tmp1[k][j * NARR_ROW + i];
                narr_hgt2[k][(j - min_jay) * num_eyes + (i - min_eye)]
                    = hgt2[k][j * NARR_ROW + i];
                narr_shum2[k][(j - min_jay) * num_eyes + (i - min_eye)]
                    = shum2[k][j * NARR_ROW + i];
                narr_tmp2[k][(j - min_jay) * num_eyes + (i - min_eye)]
                    = tmp2[k][j * NARR_ROW + i];
            }
        }
    }

    /* Release memory */
    status = free_2d_array ((void **) eye);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: eye\n", "first_files");
    }

    status = free_2d_array ((void **) jay);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: jay\n", "first_files");
    }

    status = free_2d_array ((void **) lat);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: lat\n", "first_files");
    }

    status = free_2d_array ((void **) lon);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: lon\n", "first_files");
    }

    status = free_2d_array ((void **) hgt1);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: hgt1\n", "first_files");
    }

    status = free_2d_array ((void **) shum1);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: shum1\n", "first_files");
    }

    status = free_2d_array ((void **) tmp1);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: tmp1\n", "first_files");
    }

    status = free_2d_array ((void **) hgt2);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: ght2\n", "first_files");
    }

    status = free_2d_array ((void **) shum2);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: shum2\n", "first_files");
    }

    status = free_2d_array ((void **) tmp2);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: tmp2\n", "first_files");
    }

    /* Allocate memory */
    pressure = (float **) allocate_2d_array (P_LAYER, *num_points,
                                             sizeof (float));
    if (pressure == NULL)
    {
        ERROR_MESSAGE ("Allocating pressure memory", "first_files");
    }

    narr_height1 = (float **) allocate_2d_array (P_LAYER, *num_points,
                                                 sizeof (float));
    if (narr_height1 == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_height1 memory", "first_files");
    }

    narr_height2 = (float **) allocate_2d_array (P_LAYER, *num_points,
                                                 sizeof (float));
    if (narr_height2 == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_height2 memory", "first_files");
    }

    narr_rh1 = (float **) allocate_2d_array (P_LAYER, *num_points,
                                             sizeof (float));
    if (narr_rh1 == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_rh1 memory", "first_files");
    }

    narr_rh2 = (float **) allocate_2d_array (P_LAYER, *num_points,
                                             sizeof (float));
    if (narr_rh2 == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_rh2 memory", "first_files");
    }

    for (i = 0; i < P_LAYER; i++)
    {
        for (j = 0; j < *num_points; j++)
        {
            pressure[i][j] = p[i];
        }
    }

    /* convert grib data to variables to be input to MODTRAN */
    status = convert_geopotential_geometric (*num_points, narr_lat, narr_hgt1,
                                             narr_height1);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Calling convert_geopotential_geometric1",
                       "first_files");
    }

    status = convert_geopotential_geometric (*num_points, narr_lat, narr_hgt2,
                                             narr_height2);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Calling convert_geopotential_geometric2",
                       "first_files");
    }

    status = convert_sh_rh (*num_points, narr_lat, narr_shum1, narr_tmp1,
                            pressure, narr_rh1);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Calling convert_sh_rh1", "first_files");
    }

    status = convert_sh_rh (*num_points, narr_lat, narr_shum2, narr_tmp2,
                            pressure, narr_rh2);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Calling convert_sh_rh1", "first_files");
    }

    /* free allocated memory */
    status = free_2d_array ((void **) narr_hgt1);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_hgt1\n", "first_files");
    }

    status = free_2d_array ((void **) narr_hgt2);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_hgt2\n", "first_files");
    }

    status = free_2d_array ((void **) narr_shum1);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_shum1\n", "first_files");
    }

    status = free_2d_array ((void **) narr_shum2);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_shum2\n", "first_files");
    }

    /* determine three hour-increment before and after scene center scan time */
    rem1 = input->meta.acq_date.hour % 3;
    rem2 = 3 - rem1;
    hour1 = (float) (input->meta.acq_date.hour - rem1);
    hour2 = (float) (input->meta.acq_date.hour + rem2);

    /* Round to the nearest minute */
    if ((input->meta.acq_date.second - 30.0) >= MINSIGMA)
        input->meta.acq_date.minute++;

    /* convert hour-min acquisition time to decimal time */
    time = (float) input->meta.acq_date.hour
        + (float) input->meta.acq_date.minute / 60.0;

    /* Allocate memory */
    narr_height = (float **) allocate_2d_array (P_LAYER, *num_points,
                                                sizeof (float));
    if (narr_height == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_height memory", "first_files");
    }

    /* Allocate memory */
    narr_rh = (float **) allocate_2d_array (P_LAYER, *num_points,
                                            sizeof (float));
    if (narr_rh == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_rh memory", "first_files");
    }

    /* Allocate memory */
    narr_tmp = (float **) allocate_2d_array (P_LAYER, *num_points,
                                             sizeof (float));
    if (narr_tmp == NULL)
    {
        ERROR_MESSAGE ("Allocating narr_tmp memory", "first_files");
    }

    /* linearly interpolate geometric height, relative humidity, and
       temperature for NARR points within Landsat scene this is the NARR data
       corresponding to the acquisition time of the Landsat image converted to
       appropriated variable for MODTRAN input */
    for (i = 0; i < P_LAYER; i++)
    {
        for (j = 0; j < *num_points; j++)
        {
            narr_height[i][j] = narr_height1[i][j] + (time - hour1) *
                ((narr_height2[i][j] - narr_height1[i][j]) / (hour2 - hour1));
            narr_rh[i][j] = narr_rh1[i][j] + (time - hour1) *
                ((narr_rh2[i][j] - narr_rh1[i][j]) / (hour2 - hour1));
            narr_tmp[i][j] = narr_tmp1[i][j] + (time - hour1) *
                ((narr_tmp2[i][j] - narr_tmp1[i][j]) / (hour2 - hour1));
        }
    }

    /* Free allocated memory */
    status = free_2d_array ((void **) narr_height1);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_height1\n", "first_files");
    }

    status = free_2d_array ((void **) narr_height2);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_height2\n", "first_files");
    }

    status = free_2d_array ((void **) narr_rh1);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_rh1\n", "first_files");
    }

    status = free_2d_array ((void **) narr_rh2);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_rh2\n", "first_files");
    }

    status = free_2d_array ((void **) narr_tmp1);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_tmp1\n", "first_files");
    }

    status = free_2d_array ((void **) narr_tmp2);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_tmp2\n", "first_files");
    }

    /* Build tape 5 files */
    /* Allocate memory */
    stan_height = (float *) malloc (STAN_LAYER * sizeof (float));
    if (stan_height == NULL)
    {
        ERROR_MESSAGE ("Allocating stan_height memory", "first_files");
    }

    stan_pre = (float *) malloc (STAN_LAYER * sizeof (float));
    if (stan_pre == NULL)
    {
        ERROR_MESSAGE ("Allocating stan_pre memory", "first_files");
    }

    stan_temp = (float *) malloc (STAN_LAYER * sizeof (float));
    if (stan_temp == NULL)
    {
        ERROR_MESSAGE ("Allocating stan_temp memory", "first_files");
    }

    stan_rh = (float *) malloc (STAN_LAYER * sizeof (float));
    if (stan_rh == NULL)
    {
        ERROR_MESSAGE ("Allocating stan_rh memory", "first_files");
    }

    /* read in file containing standard mid lat summer atmosphere information
       to be used for upper layers */
    sprintf (full_path, "%s/%s", path, "std_mid_lat_summer_atmos.txt");
    fd = fopen (full_path, "r");
    if (fd == NULL)
    {
        ERROR_MESSAGE ("Opening file: std_mid_lat_summer_atmos.txt\n",
                       "first_files");
    }

    for (i = 0; i < STAN_LAYER; i++)
    {
        if (fscanf (fd, "%f %f %f %f", &stan_height[i], &stan_pre[i],
                    &stan_temp[i], &stan_rh[i]) == EOF)
        {
            ERROR_MESSAGE ("End of file (EOF) is met before STAN_LAYER lines",
                           "first_files");
        }
    }

    status = fclose (fd);
    if (status)
    {
        ERROR_MESSAGE ("Closing file: std_mid_lat_summer_atmos.txt\n",
                       "first_files");
    }

    /* determine number of MODTRAN runs */
    num_cases = *num_points * NUM_ELEVATIONS * 3;

    /* Allocate memory */
    counter = (int *) malloc (STAN_LAYER * sizeof (int));
    if (counter == NULL)
    {
        ERROR_MESSAGE ("Allocating counter memory", "first_files");
    }

    case_list = (char **) allocate_2d_array (num_cases, MAX_STR_LEN,
                                             sizeof (char));
    if (case_list == NULL)
    {
        ERROR_MESSAGE ("Allocating case_list memory", "first_files");
    }

    command_list = (char **) allocate_2d_array (num_cases, MAX_STR_LEN,
                                                sizeof (char));
    if (command_list == NULL)
    {
        ERROR_MESSAGE ("Allocating command_list memory", "first_files");
    }

    modtran_path = getenv ("MODTRAN_PATH");
    if (modtran_path == NULL)
    {
        ERROR_MESSAGE ("MODTRAN_PATH environment variable is not set",
                       "first_files");
    }

    modtran_data_dir = getenv ("MODTRAN_DATA_DIR");
    if (modtran_data_dir == NULL)
    {
        ERROR_MESSAGE ("MODTRAN_DATA_DIR environment variable is not set",
                       "first_files");
    }

    getcwd (curr_path, MAX_STR_LEN);

    for (i = 0; i < *num_points; i++)
    {
        /* create a directory for the current NARR point */
        if (narr_lon[i] < 0)
            narr_lon[i] = -narr_lon[i];
        else
            narr_lon[i] = 360.0 - narr_lon[i];

        if ((fabs (narr_lon[i]) - 100.0) < MINSIGMA)
            sprintf (current_point, "%6.3f_%6.3f", narr_lat[i], narr_lon[i]);
        else
            sprintf (current_point, "%6.3f_%6.2f", narr_lat[i], narr_lon[i]);

        if (stat (current_point, &s) == -1)
        {
            status = mkdir (current_point, 0755);
            if (status != SUCCESS)
            {
                ERROR_MESSAGE ("system call 1", "first_files");
            }
        }

        /* set lowest altitude is the first geometric height at that NARR
           point (if positive) and (if negative set to zero) */
        if (narr_height[0][i] < 0)
            gndalt[0] = 0.0;
        else
            gndalt[0] = narr_height[0][i];

        /* determine latitude and longitude of current NARR point and insert
           into tail file */
        if (fabs (narr_lat[i] - 100.0) < MINSIGMA)
            sprintf (command, "cat %s/modtran_tail.txt"
                     " | sed 's/latitu/%6.3f/'"
                     " > newTail.txt", path, narr_lat[i]);
        else
            sprintf (command, "cat %s/modtran_tail.txt"
                     " | sed 's/latitu/%6.2f/'"
                     " > newTail.txt", path, narr_lat[i]);
        status = system (command);
        if (status != SUCCESS)
        {
            ERROR_MESSAGE ("system call 2", "first_file");
        }

        if (fabs (narr_lon[i] - 100.0) < MINSIGMA)
            sprintf (command, "cat newTail.txt"
                     " | sed 's/longit/%6.3f/'"
                     " > newTail2.txt", narr_lon[i]);
        else
            sprintf (command, "cat newTail.txt"
                     " | sed 's/longit/%6.2f/'"
                     " > newTail2.txt", narr_lon[i]);
        status = system (command);
        if (status != SUCCESS)
        {
            ERROR_MESSAGE ("system call 3", "first_file");
        }

        /* insert current julian day into tail file */
        sprintf (command, "cat newTail2.txt"
                 " | sed 's/jay/%d/'"
                 " > newTail3.txt", input->meta.acq_date.doy);
        status = system (command);
        if (status != SUCCESS)
        {
            ERROR_MESSAGE ("system call 4", "first_file");
        }

        /* Allocate memory */
        temp_height = (float *) malloc (MAX_MODTRAN_LAYER * sizeof (float));
        if (temp_height == NULL)
        {
            ERROR_MESSAGE ("Allocating temp_height memory", "first_files");
        }

        temp_pressure = (float *) malloc (MAX_MODTRAN_LAYER * sizeof (float));
        if (temp_pressure == NULL)
        {
            ERROR_MESSAGE ("Allocating temp_pressure memory", "first_files");
        }

        temp_temp = (float *) malloc (MAX_MODTRAN_LAYER * sizeof (float));
        if (temp_temp == NULL)
        {
            ERROR_MESSAGE ("Allocating temp_temp memory", "first_files");
        }

        temp_rh = (float *) malloc (MAX_MODTRAN_LAYER * sizeof (float));
        if (temp_rh == NULL)
        {
            ERROR_MESSAGE ("Allocating temp_rh memory", "first_files");
        }

        /* iterature through all ground altitudes at which modtran is run */
        for (j = 0; j < NUM_ELEVATIONS; j++)
        {
            /* create a directory for the current height */
            sprintf (current_gdalt, "%s/%5.3f", current_point, gndalt[j]);

            if (stat (current_gdalt, &s) == -1)
            {
                status = mkdir (current_gdalt, 0755);
                if (status != SUCCESS)
                {
                    ERROR_MESSAGE ("system call 4", "first_file");
                }
            }

            /* determine layers below current gndalt and closest index
               above and below */
            for (k = 0; k < P_LAYER; k++)
            {
                if ((narr_height[k][i] - gndalt[j]) >= MINSIGMA)
                {
                    index_below = k - 1;
                    index_above = k;
                    break;
                }
            }

            if (index_below < 0)
            {
                index_below = 0;
                index_above = 1;
            }

            /* linearly interpolate pressure, temperature, and relative 
               humidity to gndalt for lowest layer */
            new_pressure = pressure[index_below][i] + (gndalt[j] -
                                                       narr_height
                                                       [index_below][i]) *
                ((pressure[index_above][i] -
                  pressure[index_below][i]) / (narr_height[index_above][i] -
                                               narr_height[index_below][i]));
            new_temp =
                narr_tmp[index_below][i] + (gndalt[j] -
                                            narr_height[index_below][i]) *
                ((narr_tmp[index_above][i] -
                  narr_tmp[index_below][i]) / (narr_height[index_above][i] -
                                               narr_height[index_below][i]));
            new_rh =
                narr_rh[index_below][i] + (gndalt[j] -
                                           narr_height[index_below][i]) *
                ((narr_rh[index_above][i] -
                  narr_rh[index_below][i]) / (narr_height[index_above][i] -
                                              narr_height[index_below][i]));

            /* create arrays containing only layers to be included in current
               tape5 file */
            index = 0;
            temp_height[index] = gndalt[j];
            temp_pressure[index] = new_pressure;
            temp_temp[index] = new_temp;
            temp_rh[index] = new_rh;
            index++;

            for (k = index_above; k < P_LAYER; k++)
            {
                temp_height[index] = narr_height[k][i];
                temp_pressure[index] = pressure[k][i];
                temp_temp[index] = narr_tmp[k][i];
                temp_rh[index] = narr_rh[k][i];
                index++;
            }


            /* modtran throws as error when there are two identical layers in
               the tape5 file, if the current ground altitude and the next
               highest layer are close enough, eliminate interpolated layer */
            if (fabs (gndalt[j] - narr_height[index_above][i] - 0.001)
                < MINSIGMA)
            {
                index = 0;
                for (k = index_above; k < P_LAYER; k++)
                {
                    temp_height[index] = narr_height[k][i];
                    temp_pressure[index] = pressure[k][i];
                    temp_temp[index] = narr_tmp[k][i];
                    temp_rh[index] = narr_rh[k][i];
                    index++;
                }
            }

            /* determine maximum height of NARR layers and where the standard 
               atmosphere is greater than this */
            index2 = 0;
            for (k = 0; k < STAN_LAYER; k++)
            {
                if (stan_height[k] > narr_height[P_LAYER - 1][i])
                {
                    counter[index2] = k;
                    index2++;
                }
            }

            /* if there are at least three layers above to highest NARR layer,
               add standard atmosphere layers and linearly interpolate height,
               pressure, temp, and rel hum to create a smooth transition
               between the NARR layers and the standard upper atmosphere */
// RDD - Did this to remove a compiler warning, but something else may need to be done.
            new_height = 0;
            if (index2 >= 3)
            {
                new_height =
                    (stan_height[counter[2]] + temp_height[index - 1]) / 2.0;
                new_pressure =
                    temp_pressure[index - 1] + (new_height -
                                                temp_height[index -
                                                            1]) *
                    ((stan_pre[counter[2]] -
                      temp_pressure[index - 1]) / (stan_height[counter[2]] -
                                                   temp_height[index - 1]));
                new_temp =
                    temp_temp[index - 1] + (new_height -
                                            temp_height[index -
                                                        1]) *
                    ((stan_temp[counter[2]] -
                      temp_temp[index - 1]) / (stan_height[counter[2]] -
                                               temp_height[index - 1]));
                new_rh =
                    temp_rh[counter[2]] + (new_height -
                                           temp_height[index -
                                                       1]) *
                    ((stan_rh[counter[2]] -
                      temp_rh[index - 1]) / (stan_height[counter[2]] -
                                             temp_height[index - 1]));
            }

            /* concatenate NARR layers, new layer, and standard atmosphere
               layers */
            temp_height[index] = new_height;
            temp_pressure[index] = new_pressure;
            temp_temp[index] = new_temp;
            temp_rh[index] = new_rh;
            index++;
            for (k = 2; k < index2; k++)
            {
                temp_height[index] = stan_height[counter[k]];
                temp_pressure[index] = stan_pre[counter[k]];
                temp_temp[index] = stan_temp[counter[k]];
                temp_rh[index] = stan_rh[counter[k]];
                index++;
            }

            /* write atmospheric layers to a text file in format proper for
               tape5 file */
            fd = fopen ("tempLayers.txt", "w");
            if (fd == NULL)
            {
                ERROR_MESSAGE ("Opening file: tempLayers.txt\n",
                               "first_files");
            }

            /* Write out the intermediate file */
            for (k = 0; k < index; k++)
            {
                fprintf (fd, "%10.3f%10.3e%10.3e%10.3e%10.3e%10.3e%16s\n",
                         temp_height[k], temp_pressure[k], temp_temp[k],
                         temp_rh[k], 0.0, 0.0, "AAH             ");
            }

            /* Close the intermediate file */
            status = fclose (fd);
            if (status)
            {
                ERROR_MESSAGE ("Closing file: tempLayers.txt\n",
                               "first_files");
            }

            /* determine number of layers for current ground altitude and
               insert into head file */
            sprintf (command, "cat %s/modtran_head.txt"
                     " | sed 's/nml/%d/'" " > newHead.txt", path, index);

            status = system (command);
            if (status != SUCCESS)
            {
                ERROR_MESSAGE ("system call 5", "first_file");
            }

            /* insert current ground altitude into head file */
            sprintf (command, "cat newHead.txt"
                     " | sed 's/gdalt/%5.3f/'" " > newHead2.txt", gndalt[j]);
            status = system (command);
            if (status != SUCCESS)
            {
                ERROR_MESSAGE ("system call 6", "first_file");
            }

            /* iterate through [temperature,albedo] pairs at which to run
               MODTRAN */
            for (k = 0; k <= 2; k++)
            {
                if (k == 2)
                    sprintf (temp_out, "%s", "000");
                else
                    sprintf (temp_out, "%d", (int) tmp[k]);

                /* create directory for the current temperature */
                sprintf (current_temp, "%s/%s", current_gdalt, temp_out);

                if (stat (current_temp, &s) == -1)
                {
                    status = mkdir (current_temp, 0755);
                    if (status != SUCCESS)
                    {
                        ERROR_MESSAGE ("system call 7", "first_file");
                    }
                }

                /* insert current temperature into head file */
                sprintf (command, "cat newHead2.txt"
                         " | sed 's/tmp/%s/'" " > newHead3.txt", temp_out);
                status = system (command);
                if (status != SUCCESS)
                {
                    ERROR_MESSAGE ("system call 8", "first_file");
                }

                /* create directory for the current albedo */
                sprintf (current_alb, "%s/%3.1f", current_temp, alb[k]);
                if (stat (current_alb, &s) == -1)
                {
                    status = mkdir (current_alb, 0755);
                    if (status != SUCCESS)
                    {
                        ERROR_MESSAGE ("system call 9", "first_file");
                    }
                }

                /* insert current albedo into head file */
                sprintf (command, "cat newHead3.txt"
                         " | sed 's/alb/%4.2f/'" " > newHead4.txt", alb[k]);
                status = system (command);
                if (status != SUCCESS)
                {
                    ERROR_MESSAGE ("system call 10", "first_file");
                }

                /* concatenate head file, atmospheric layers, and tail file to
                   create a tape5 file for modtran specific to this location
                   and ground altitude with variables for temperature and
                   albedo */
                sprintf (command, "cat newHead4.txt"
                         " tempLayers.txt"
                         " newTail3.txt" " > %s/tape5", current_alb);
                status = system (command);
                if (status != SUCCESS)
                {
                    ERROR_MESSAGE ("system call 11", "first_file");
                }

                /* create string for case list containing location of current
                   tape5 file
                   create string for command list containing commands for
                   modtran run
                   iterate entry count */
                case_counter = i * NUM_ELEVATIONS * 3 + j * 3 + k;
                sprintf (case_list[case_counter], "%s/%s",
                         curr_path, current_alb);
                sprintf (command_list[case_counter],
                         "pushd %s; ln -s %s; %s/Mod90_5.2.2.exe; popd",
                         case_list[case_counter], modtran_data_dir,
                         modtran_path);
            }
        }
    }

    /* write caseList to a file */
    fd = fopen ("caseList", "w");
    if (fd == NULL)
    {
        ERROR_MESSAGE ("Opening file: caseList\n", "first_files");
    }

    /* Write out the caseList file */
    for (k = 0; k < *num_points * NUM_ELEVATIONS * 3; k++)
    {
        fprintf (fd, "%s\n", case_list[k]);
    }

    /* Close the caseList file */
    status = fclose (fd);
    if (status)
    {
        ERROR_MESSAGE ("Closing file: caseList\n", "first_files");
    }

    /* write commandList to a file */
    fd = fopen ("commandList", "w");
    if (fd == NULL)
    {
        ERROR_MESSAGE ("Opening file: commandList\n", "first_files");
    }

    /* Write out the commandList file */
    for (k = 0; k < *num_points * NUM_ELEVATIONS * 3; k++)
    {
        fprintf (fd, "%s\n", command_list[k]);
    }

    /* Close the commandList file */
    status = fclose (fd);
    if (status)
    {
        ERROR_MESSAGE ("Closing file: commandList\n", "first_files");
    }

    /* Free memory allocation */
    status = free_2d_array ((void **) pressure);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: pressure\n", "first_files");
    }

    status = free_2d_array ((void **) narr_height);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_height\n", "first_files");
    }

    status = free_2d_array ((void **) narr_rh);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_rh\n", "first_files");
    }

    status = free_2d_array ((void **) narr_tmp);
    if (status != SUCCESS)
    {
        ERROR_MESSAGE ("Freeing memory: narr_tmp\n", "first_files");
    }

    /* Free memory allocation */
    status = free_2d_array ((void **) case_list);
    if (status != SUCCESS)
    {
        RETURN_ERROR ("Freeing memory: command_list\n", "scene_based_list",
                      FAILURE);
    }

    status = free_2d_array ((void **) command_list);
    if (status != SUCCESS)
    {
        RETURN_ERROR ("Freeing memory: command_list\n", "scene_based_list",
                      FAILURE);
    }

    return SUCCESS;
}
