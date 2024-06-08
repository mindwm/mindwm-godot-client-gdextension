extends Node2D


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	$Xorg.init()
	$Xorg.refresh_xorg_windows()
	for w in $Xorg.list_windows():
		print("Win: %s (%s) (%s)" % [w.get_name(), w.get_class(), w.get_rect()])


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
