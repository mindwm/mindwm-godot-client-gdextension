extends Node2D


# Called when the node enters the scene tree for the first time.
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

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass


func _on_xorg_window_created(window_info: Object) -> void:
	print("event window_created received: %s" % window_info)
	$RootUI/PanelContainer/ItemList.add_item("%s (%d)" % [
		window_info.get_name(),
		window_info.get_win_id()
		])


func _on_xorg_window_destroyed(window_info: Object) -> void:
	print("event window_destroyed received: %s" % window_info)
	var ilist : ItemList = $RootUI/PanelContainer/ItemList
	var t = "%s (%d)" % [
		window_info.get_name(),
		window_info.get_win_id()
		]
	for i in ilist.item_count:
		if ilist.get_item_text(i) == t:
			ilist.remove_item(i)
		
