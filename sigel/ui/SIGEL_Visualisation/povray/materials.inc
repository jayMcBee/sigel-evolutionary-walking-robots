#declare sigelPaintFinish = finish {
  ambient 0
  diffuse 0.85
  specular 0.3
  roughness 0.01
  reflection { 0.01, 0.1 falloff 3 }
  conserve_energy
}

#declare sigelSteelColor = <0.42, 0.42, 0.43>;

#declare sigelSteelFinish = finish {
  ambient 0
  diffuse 0.5
  specular 0.5
  roughness 0.02
  metallic
  reflection { 0.1, 0.25 metallic }
  conserve_energy
}

// A robot material: paint over steel. Where the noise pattern is high, the paint is worn
// thin: the colour is 70% paint and 30% steel, with the steel's finish, so that the worn
// patches stay close to the paint on dark and light colours alike. The map's textures take
// on the pattern's scale, so the paint's bumps, at 7/30 of it, are sigelPatternSize / 30
// across.
#macro sigelPaint( paintColor )
  texture {
    bozo
    turbulence 1
    scale sigelPatternSize / 7
    texture_map {
      [0.00 pigment { srgb paintColor } normal { bumps 0.015 scale 0.2333 } finish { sigelPaintFinish }]
      [0.66 pigment { srgb paintColor } normal { bumps 0.015 scale 0.2333 } finish { sigelPaintFinish }]
      [0.76 pigment { srgb paintColor * 0.7 + sigelSteelColor * 0.3 } finish { sigelSteelFinish }]
      [1.00 pigment { srgb paintColor * 0.7 + sigelSteelColor * 0.3 } finish { sigelSteelFinish }]
    }
  }
#end

