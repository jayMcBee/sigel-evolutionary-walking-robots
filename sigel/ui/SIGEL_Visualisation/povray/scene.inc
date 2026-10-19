// The key light is the screen's sun. The fill light's colour is linear: a quarter of
// srgb <0.75, 0.82, 1>, since POV-Ray scales an srgb colour before it decodes it.
light_source {
  sigelSunPosition
  srgb <1, 0.96, 0.9>
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
