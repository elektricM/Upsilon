apps += Rpi::App
app_headers += apps/rpi/app.h

app_srpi_src = $(addprefix apps/rpi/,\
  app.cpp \
  rpi_controller.cpp \
  rpi_view.cpp \
)

apps_src += $(app_srpi_src)

app_images += apps/rpi/rpi_icon.png

i18n_files += $(call i18n_without_universal_for,rpi/base)

$(eval $(call depends_on_image,apps/rpi/app.cpp,apps/rpi/rpi_icon.png))
