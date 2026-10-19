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

