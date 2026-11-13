extends Control

const ESP32_IP := "192.168.4.1"
const ESP32_PORT := 5000

var tcp := StreamPeerTCP.new()
var is_connected_to_device := false
var rx_buffer := ""
var last_ping_ms := 0

var status_label: Label
var humidity_label: Label
var valve_label: Label
var next_watering_label: Label
var event_label: Label
var min_humidity_spin: SpinBox
var high_humidity_spin: SpinBox
var water_ml_spin: SpinBox
var interval_spin: SpinBox
var manual_ml_spin: SpinBox
var connect_button: Button
var disconnect_button: Button

func _ready() -> void:
    _build_ui()
    set_process(true)

func _build_ui() -> void:
    var background := ColorRect.new()
    background.color = Color("#101a17")
    background.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    add_child(background)

    var scroll := ScrollContainer.new()
    scroll.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    scroll.offset_left = 28
    scroll.offset_top = 24
    scroll.offset_right = -28
    scroll.offset_bottom = -24
    add_child(scroll)

    var column := VBoxContainer.new()
    column.custom_minimum_size = Vector2(560, 0)
    column.add_theme_constant_override("separation", 14)
    scroll.add_child(column)

    var title := Label.new()
    title.text = "🌱  Maceta inteligente"
    title.add_theme_font_size_override("font_size", 30)
    column.add_child(title)

    status_label = _label("Desconectado — conecta el PC a MACETA_ESP32")
    column.add_child(status_label)

    connect_button = Button.new()
    connect_button.text = "Conectar con ESP32"
    connect_button.pressed.connect(connect_device)
    column.add_child(connect_button)

    disconnect_button = Button.new()
    disconnect_button.text = "Finalizar sesión y dormir ESP32"
    disconnect_button.pressed.connect(disconnect_device)
    disconnect_button.disabled = true
    column.add_child(disconnect_button)

    column.add_child(_section_title("Estado de la planta"))
    humidity_label = _label("Humedad: -- %")
    valve_label = _label("Electroválvula: --")
    next_watering_label = _label("Próximo riego: --")
    event_label = _label("Evento: --")
    column.add_child(humidity_label)
    column.add_child(valve_label)
    column.add_child(next_watering_label)
    column.add_child(event_label)

    column.add_child(_section_title("Configuración"))
    min_humidity_spin = _spin(1, 95, 30, 1)
    high_humidity_spin = _spin(2, 99, 65, 1)
    water_ml_spin = _spin(1, 1000, 100, 10)
    interval_spin = _spin(1, 720, 24, 1)
    _add_setting(column, "Umbral de suelo seco (%)", min_humidity_spin)
    _add_setting(column, "Umbral de humedad alta (%)", high_humidity_spin)
    _add_setting(column, "Agua estimada por riego (ml)", water_ml_spin)
    _add_setting(column, "Intervalo programado (horas)", interval_spin)

    var save_button := Button.new()
    save_button.text = "Guardar configuración en ESP32"
    save_button.pressed.connect(send_config)
    column.add_child(save_button)

    column.add_child(_section_title("Riego manual"))
    manual_ml_spin = _spin(1, 1000, 50, 10)
    _add_setting(column, "Cantidad solicitada (ml)", manual_ml_spin)

    var manual_button := Button.new()
    manual_button.text = "Solicitar riego manual"
    manual_button.pressed.connect(request_manual_watering)
    column.add_child(manual_button)

    var note := _label("El ESP32 aplica límites de seguridad. El riego puede ser rechazado si el suelo está húmedo o el driver no está habilitado.")
    note.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    column.add_child(note)

func _label(value: String) -> Label:
    var label := Label.new()
    label.text = value
    label.add_theme_font_size_override("font_size", 16)
    return label

func _section_title(value: String) -> Label:
    var label := _label(value)
    label.add_theme_font_size_override("font_size", 22)
    return label

func _spin(min_value: float, max_value: float, initial: float, step: float) -> SpinBox:
    var spin := SpinBox.new()
    spin.min_value = min_value
    spin.max_value = max_value
    spin.value = initial
    spin.step = step
    spin.custom_minimum_size.x = 150
    return spin

func _add_setting(parent: VBoxContainer, caption: String, control: Control) -> void:
    var row := HBoxContainer.new()
    var label := _label(caption)
    label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    row.add_child(label)
    row.add_child(control)
    parent.add_child(row)

func connect_device() -> void:
    tcp = StreamPeerTCP.new()
    var error := tcp.connect_to_host(ESP32_IP, ESP32_PORT)
    if error != OK:
        status_label.text = "No se pudo iniciar la conexión: %s" % error
        return
    status_label.text = "Conectando a %s:%d..." % [ESP32_IP, ESP32_PORT]
    connect_button.disabled = true

func disconnect_device() -> void:
    if tcp.get_status() == StreamPeerTCP.STATUS_CONNECTED:
        _send_json({"type": "shutdown"})
    tcp.disconnect_from_host()
    _set_disconnected("Sesión finalizada")

func send_config() -> void:
    _send_json({
        "type": "config",
        "min_humidity": min_humidity_spin.value,
        "high_humidity": high_humidity_spin.value,
        "water_ml": water_ml_spin.value,
        "interval_hours": int(interval_spin.value)
    })

func request_manual_watering() -> void:
    _send_json({
        "type": "manual_watering",
        "water_ml": manual_ml_spin.value
    })

func _send_json(payload: Dictionary) -> void:
    if tcp.get_status() != StreamPeerTCP.STATUS_CONNECTED:
        status_label.text = "No hay conexión con la maceta"
        return
    var line := JSON.stringify(payload) + "\n"
    var result := tcp.put_data(line.to_utf8_buffer())
    if result != OK:
        status_label.text = "Error enviando datos: %s" % result

func _process(_delta: float) -> void:
    tcp.poll()
    var current_status := tcp.get_status()

    if current_status == StreamPeerTCP.STATUS_CONNECTED:
        if not is_connected_to_device:
            is_connected_to_device = true
            status_label.text = "Conectado a la maceta"
            connect_button.disabled = true
            disconnect_button.disabled = false
        _read_messages()
        var now := Time.get_ticks_msec()
        if now - last_ping_ms >= 30000:
            _send_json({"type": "ping"})
            last_ping_ms = now
    elif is_connected_to_device:
        _set_disconnected("Conexión perdida")
    elif current_status == StreamPeerTCP.STATUS_ERROR:
        status_label.text = "No se pudo conectar. Comprueba que el ESP32 esté despierto y que el PC esté en su Wi-Fi."
        connect_button.disabled = false

func _read_messages() -> void:
    var available := tcp.get_available_bytes()
    if available <= 0:
        return

    var result := tcp.get_data(available)
    if result[0] != OK:
        return

    rx_buffer += result[1].get_string_from_utf8()
    while rx_buffer.contains("\n"):
        var index := rx_buffer.find("\n")
        var line := rx_buffer.substr(0, index).strip_edges()
        rx_buffer = rx_buffer.substr(index + 1)
        if not line.is_empty():
            _handle_message(line)

func _handle_message(line: String) -> void:
    var parsed = JSON.parse_string(line)
    if typeof(parsed) != TYPE_DICTIONARY:
        return

    match parsed.get("type", ""):
        "telemetry":
            var humidity = parsed.get("humidity", -1.0)
            humidity_label.text = "Humedad: " + ("--" if humidity == null else str(snappedf(float(humidity), 0.1))) + "%"
            valve_label.text = "Electroválvula: %s" % ("Abierta" if parsed.get("valve", false) else "Cerrada")
            next_watering_label.text = "Próximo riego: %s min" % str(parsed.get("next_watering_minutes", "--"))
            event_label.text = "Evento: %s" % str(parsed.get("event", "--"))
        "ack":
            event_label.text = "%s: %s" % [str(parsed.get("event", "respuesta")), str(parsed.get("message", ""))]

func _set_disconnected(message: String) -> void:
    is_connected_to_device = false
    connect_button.disabled = false
    disconnect_button.disabled = true
    status_label.text = message

func _notification(what: int) -> void:
    if what == NOTIFICATION_WM_CLOSE_REQUEST:
        if tcp.get_status() == StreamPeerTCP.STATUS_CONNECTED:
            _send_json({"type": "shutdown"})
        tcp.disconnect_from_host()
        get_tree().quit()
