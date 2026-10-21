// The key light is the screen's sun. Both colours are linear, since POV-Ray scales an
// srgb colour before it decodes it: the key is 1.4 times srgb <1, 0.96, 0.9>, the fill a
// quarter of srgb <0.75, 0.82, 1>.
light_source {
  sigelSunPosition
  rgb <1.4, 1.28, 1.1>
  area_light 100*x, 100*z, 9, 9
  adaptive 1
  circular
  orient
}

light_source {
  sigelFillPosition
  rgb <0.131, 0.16, 0.25>
  area_light 150*x, 150*z, 5, 5
  adaptive 1
  circular
  orient
}

sky_sphere {
  pigment {
    gradient y
    color_map {
      [0.5 srgb sigelHorizonColor]
      [1.0 srgb sigelSkyTopColor]
    }
    scale 2
    translate -y
  }
}

// Without a texture, a floor of one colour shows no motion while the camera follows the
// robot. So it gets a faint grain, 11 percent either side of its colour: two noise patterns
// that scale with the robot. The 11 percent is a tuning constant from sample renders.
#declare sigelGrainColorMap = color_map {
  [0 srgb sigelFloorColor * 0.89]
  [1 srgb sigelFloorColor * 1.11]
}

// The image fills the square from 0 to 1 in x and y. Laid flat, its top row points to
// SIGEL +z, as in the OpenGL renderer.
#declare Plane = object {
  Ground
  texture {
    #ifdef (sigelFloorImage)
      pigment {
        image_map { png sigelFloorImage interpolate 2 }
        scale <sigelFloorImageWidth, sigelFloorTileSize, 1>
        rotate x*90
      }
    #else
      pigment {
        average
        pigment_map {
          [1 bozo color_map { sigelGrainColorMap } scale sigelPatternSize]
          [1 granite color_map { sigelGrainColorMap } scale sigelPatternSize / 2]
        }
      }
    #end
    // A tuning constant from sample renders: at diffuse 1, the sun washes the floor out.
    finish { ambient 0 diffuse 0.8 }
  }
}

// One layer of grid lines across axis: a pattern, so that it has no end. One line every grid
// spacing, as wide as the robot's path.
#macro sigelGridLines( axis )
  #local halfWidth = sigelLineRadius / sigelGridSpacing;
  texture {
    pigment {
      gradient axis
      color_map {
        [0 srgb sigelGridColor]
        [halfWidth srgb sigelGridColor]
        [halfWidth srgbt <0,0,0,1>]
        [1 - halfWidth srgbt <0,0,0,1>]
        [1 - halfWidth srgb sigelGridColor]
        [1 srgb sigelGridColor]
      }
      scale sigelGridSpacing
    }
    finish { ambient 0 diffuse 1 }
  }
#end

// The lines lie on a copy of the ground, raised as the OpenGL renderer raises them; the copy
// is clear between the lines.
#declare Grid = object {
  Ground
  translate <0,0.01,0>
  sigelGridLines( x )
  sigelGridLines( z )
  no_shadow
}

// The OpenGL renderer's camera: cameraDirection gives its vertical field of view.
#macro sigelCamera( eyePoint, lookPoint )
  camera {
    right x*image_width/image_height
    up y
    direction cameraDirection
    location eyePoint
    look_at lookPoint
  }
#end
