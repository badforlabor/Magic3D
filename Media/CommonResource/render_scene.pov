// This work is licensed under the Creative Commons Attribution 3.0 Unported License.
// To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
// or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View,
// California, 94041, USA.

// Persistence Of Vision Ray Tracer Scene Description File
// File: radiosity3.pov
// Desc: radiosity tutorial scene
// Date: 2000-2001
// Auth: Christoph Hormann

// -w240 -h180 +a0.3

// Updated: 29Dec2010 (cli) modified scene to use 3.7 syntax and more realistic diffuse finish

//#version 3.6;
#version 3.7;

#declare use_light=true;
#declare lightlenX = 0.408248290 * 1.2;
#declare lightlenY = 0.7071 * 1.2;

global_settings {
  assumed_gamma 1.0
  
  radiosity {
    pretrace_start 0.08
    pretrace_end   0.04
    count 1024

    nearest_count 5
    error_bound 0.2
    recursion_limit 1

    low_error_factor .5
    gray_threshold 0.0
    minimum_reuse 0.015
    brightness 1

    adc_bailout 0.01/2

  }
}

#if (use_light)
  light_source {
    <-0.3, 0.7, 1>*10
    color rgb <0.8, 0.8, 0.8>
    area_light <-1 * lightlenX, 2 * lightlenX, -1 * lightlenX>, <lightlenY, 0, -1 * lightlenY>, 17, 17
    adaptive 0
    jitter

    spotlight
    point_at <0, 0, 0>
    tightness 0
    radius 30
    falloff 60
  }
#end

camera {
  location <0, 0, 3>
  look_at <0.0, 0, 0.0>
}

sphere {
  <0, 0, 0>, 1
  texture {
   pigment {
     gradient y
     color_map {
       [0.0 color rgb < 1, 1, 1 >]
     }
   }
   finish { diffuse 0 emission 1.4 }
  }
  hollow on
  no_shadow
  scale 30000
}

#include "mesh.inc"

//plane {
//  y, -1
//  texture {
//    pigment { color rgb 1 }
//    finish { diffuse 0.65 }
//  }
//}

#include "smallplane.inc"
