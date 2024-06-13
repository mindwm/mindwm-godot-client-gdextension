extends Node2D

# Called when the node enters the scene tree for the first time.
var win_ndx : int = -1;

func _ready() -> void:
	$Xorg.init()
	$Xorg.refresh_xorg_windows()
	for w in $Xorg.list_windows():
		print("Win: %x (%x): %s (%s) (%s)" % [
			w.get_win_id(),
			w.get_parent_id(),
			w.get_name(),
			w.get_class(),
			w.get_rect()
			])

	var uiWindowList : ItemList = $RootUI/PanelContainer/WindowList
	uiWindowList.item_activated.connect(_on_window_selected)


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	if (win_ndx >= 0):
		$Xorg.capture_window_egl(win_ndx)


func _on_xorg_window_created(window_info: Object) -> void:
	print("event window_created received: %s" % window_info)
	$RootUI/PanelContainer/WindowList.add_item("%s (%d)" % [
		window_info.get_name(),
		window_info.get_win_id()
		])


func _on_xorg_window_destroyed(window_info: Object) -> void:
	print("event window_destroyed received: %s" % window_info)
	var ilist : ItemList = $RootUI/PanelContainer/WindowList
	var t = "%s (%d)" % [
		window_info.get_name(),
		window_info.get_win_id()
		]
	for i in ilist.item_count:
		if ilist.get_item_text(i) == t:
			ilist.remove_item(i)
		


func _on_xorg_window_configured(window_info: Object) -> void:
	print("event window_configured received: %s (%s)" % 
	  [window_info.get_name(),
		window_info.get_rect()
	  ])

func _on_window_selected(ndx : int) -> void:
	win_ndx = ndx
	var w = $Xorg.get_wm_window(win_ndx)
	var wt = $Xorg.get_wm_window_texture(win_ndx)
	$WindowTexture.texture = wt;
	$WindowTexture.material.set_shader_parameter("tex", wt)
