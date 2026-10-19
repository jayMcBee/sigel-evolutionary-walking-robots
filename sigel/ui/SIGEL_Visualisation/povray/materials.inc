#declare sigelPaintFinish = finish {
  ambient 0
  diffuse 0.85
  specular 0.3
  roughness 0.01
  reflection { 0.01, 0.1 falloff 3 }
  conserve_energy
}

#declare sigelSteel = texture {
  pigment { srgb <0.42, 0.42, 0.43> }
  finish {
    ambient 0
    diffuse 0.5
    specular 0.5
    roughness 0.02
    metallic
    reflection { 0.1, 0.25 metallic }
    conserve_energy
  }
}

// A robot material: paint over steel. Where the noise pattern is high, the paint is
// worn off and the steel shows. The map's textures take on the pattern's scale, so the
// paint's bumps, at 14/30 of it, are sigelPatternSize / 30 across.
#macro sigelPaint( paintColor )
  texture {
    bozo
    turbulence 1
    scale sigelPatternSize / 14
    texture_map {
      [0.00 pigment { srgb paintColor } normal { bumps 0.015 scale 0.4667 } finish { sigelPaintFinish }]
      [0.70 pigment { srgb paintColor } normal { bumps 0.015 scale 0.4667 } finish { sigelPaintFinish }]
      [0.73 sigelSteel]
      [1.00 sigelSteel]
    }
  }
#end

