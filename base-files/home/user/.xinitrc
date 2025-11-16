export NO_AT_BRIDGE=1
# mate-panel --sm-client-disable &
mate-terminal &
exec dbus-launch marco --sm-disable
