extends Control

const ESP32_IP := "192.168.4.1"
const ESP32_PORT := 5000
const WIFI_SSID := "MACETA_ESP32"
const WIFI_PASSWORD := "riego1234"

var tcp := StreamPeerTCP.new()
var connected := false
var rx_buffer := ""
var last_ping_ms := 0
var received_initial_config := false

var status_label: Label
var humidity_label: Label
var raw_label: Label
var flow_label: Label
var volume_label: Label
var valve_label: Label
var state_label: Label
var event_label: Label
var connect_button: Button
var disconnect_button: Button
var start_spin: SpinBox
var target_spin: SpinBox
var burst_spin: SpinBox
var soak_spin: SpinBox
var max_time_spin: SpinBox
var max_volume_spin: SpinBox

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
    column.custom_minimum_size = Vector2(600, 0)
    column.add_theme_constant_override("separation", 12)
    scroll.add_child(column)

    var title := _label("🌱  Mini-invernadero inteligente", 28)
    column.add_child(title)
    status_label = _label("Desconectado. Conecta el PC a la red Wi-Fi del ESP32.", 16)
    status_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    column.add_child(status_label)

    var wifi := _label("Wi-Fi: %s   Contraseña: %s   TCP: %s:%d" % [WIFI_SSID, WIFI_PASSWORD, ESP32_IP, ESP32_PORT], 13)
    wifi.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    column.add_child(wifi)

    var buttons := HBoxContainer.new()
    connect_button = Button.new()
    connect_button.text = "Conectar"
    connect_button.pressed.connect(connect_device)
    buttons.add_child(connect_button)
    disconnect_button = Button.new()
    disconnect_button.text = "Desconectar"
    disconnect_button.disabled = true
    disconnect_button.pressed.connect(disconnect_device)
    buttons.add_child(disconnect_button)
    column.add_child(buttons)

    column.add_child(_section_title("Lecturas en tiempo real"))
    humidity_label = _label("Humedad del suelo: -- %", 20)
    raw_label = _label("ADC suelo: --", 14)
    flow_label = _label("Caudal: -- L/min", 20)
    volume_label = _label("Agua suministrada en el ciclo: -- L", 18)
    valve_label = _label("Electroválvula: --", 18)
    state_label = _label("Estado: --", 18)
    event_label = _label("Evento: --", 15)
    for item in [humidity_label, raw_label, flow_label, volume_label, valve_label, state_label, event_label]:
        item.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
        column.add_child(item)

    column.add_child(_section_title("Parámetros del algoritmo de riego"))
    start_spin = _spin(1, 90, 35, 1)
    target_spin = _spin(2, 99, 45, 1)
    burst_spin = _spin(1, 10, 5, 0.5)
    soak_spin = _spin(5, 180, 30, 1)
    max_time_spin = _spin(5, 120, 30, 1)
    max_volume_spin = _spin(0.05, 2, 0.5, 0.05)
    _add_setting(column, "Iniciar riego si humedad ≤ (%)", start_spin)
    _add_setting(column, "Objetivo de humedad (%)", target_spin)
    _add_setting(column, "Duración de cada pulso (s)", burst_spin)
    _add_setting(column, "Pausa de infiltración (s)", soak_spin)
    _add_setting(column, "Tiempo máximo acumulado de válvula (s)", max_time_spin)
    _add_setting(column, "Volumen máximo por ciclo (L)", max_volume_spin)

    var save := Button.new()
    save.text = "Enviar configuración al ESP32"
    save.pressed.connect(send_config)
    column.add_child(save)

    var note := _label("El riego se decide por humedad del suelo. El caudalímetro mide el agua entregada y permite detectar ausencia de flujo. La configuración se mantiene en RAM y vuelve a los valores predeterminados al reiniciar el ESP32. El riego manual remoto está deshabilitado por seguridad.", 13)
    note.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    column.add_child(note)

func _label(value: String, size: int = 16) -> Label:
    var label := Label.new()
    label.text = value
    label.add_theme_font_size_override("font_size", size)
    return label

func _section_title(value: String) -> Label:
    return _label(value, 22)

func _spin(minimum: float, maximum: float, initial: float, step_value: float) -> SpinBox:
    var spin := SpinBox.new()
    spin.min_value = minimum
    spin.max_value = maximum
    spin.value = initial
    spin.step = step_value
    spin.custom_minimum_size.x = 150
    return spin

func _add_setting(parent: VBoxContainer, caption: String, control: Control) -> void:
    var row := HBoxContainer.new()
    var label := _label(caption, 15)
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
        tcp.disconnect_from_host()
    _set_disconnected("Sesión desconectada")

func send_config() -> void:
    if start_spin.value >= target_spin.value:
        event_label.text = "Error: el objetivo debe superar el umbral de inicio."
        return
    _send_json({
        "type": "config",
        "soil_start_percent": start_spin.value,
        "soil_target_percent": target_spin.value,
        "burst_seconds": burst_spin.value,
        "soak_seconds": soak_spin.value,
        "max_total_seconds": max_time_spin.value,
        "max_volume_l": max_volume_spin.value
    })

func _send_json(payload: Dictionary) -> void:
    if tcp.get_status() != StreamPeerTCP.STATUS_CONNECTED:
        status_label.text = "No hay conexión con el ESP32."
        return
    var result := tcp.put_data((JSON.stringify(payload) + "\n").to_utf8_buffer())
    if result != OK:
        status_label.text = "Error enviando datos: %s" % result

func _process(_delta: float) -> void:
    tcp.poll()
    var current := tcp.get_status()
    if current == StreamPeerTCP.STATUS_CONNECTED:
        if not connected:
            connected = true
            status_label.text = "Conectado al ESP32. Recibiendo telemetría."
            connect_button.disabled = true
            disconnect_button.disabled = false
            received_initial_config = false
            _send_json({"type": "get_status"})
        _read_messages()
        var now := Time.get_ticks_msec()
        if now - last_ping_ms >= 10000:
            _send_json({"type": "ping"})
            last_ping_ms = now
    elif connected:
        _set_disconnected("Conexión perdida.")
    elif current == StreamPeerTCP.STATUS_ERROR:
        status_label.text = "No se pudo conectar. Verifica que estés conectado a MACETA_ESP32."
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
            var humidity = parsed.get("soil_moisture_pct", null)
            humidity_label.text = "Humedad del suelo: %s" % ("Sin calibrar" if humidity == null else "%.1f %%" % float(humidity))
            raw_label.text = "ADC suelo: %s" % str(parsed.get("soil_raw", "--"))
            flow_label.text = "Caudal: %.3f L/min" % float(parsed.get("flow_l_min", 0.0))
            volume_label.text = "Agua suministrada en el ciclo: %.3f L" % float(parsed.get("water_liters_cycle", 0.0))
            valve_label.text = "Electroválvula: %s" % ("ABIERTA" if parsed.get("valve_open", false) else "CERRADA")
            state_label.text = "Estado: %s" % str(parsed.get("state", "--"))
            event_label.text = "Evento: %s" % str(parsed.get("event", "--"))
            _sync_config(parsed)
        "ack":
            event_label.text = "ESP32: %s" % str(parsed.get("message", "Respuesta"))
            if not parsed.get("ok", true):
                status_label.text = "El ESP32 rechazó la solicitud."

func _sync_config(data: Dictionary) -> void:
    if received_initial_config:
        return
    received_initial_config = true
    start_spin.value = float(data.get("soil_start_percent", start_spin.value))
    target_spin.value = float(data.get("soil_target_percent", target_spin.value))
    burst_spin.value = float(data.get("burst_seconds", burst_spin.value))
    soak_spin.value = float(data.get("soak_seconds", soak_spin.value))
    max_time_spin.value = float(data.get("max_total_seconds", max_time_spin.value))
    max_volume_spin.value = float(data.get("max_volume_l", max_volume_spin.value))

func _set_disconnected(message: String) -> void:
    connected = false
    connect_button.disabled = false
    disconnect_button.disabled = true
    status_label.text = message
    rx_buffer = ""

func _notification(what: int) -> void:
    if what == NOTIFICATION_WM_CLOSE_REQUEST:
        tcp.disconnect_from_host()
        get_tree().quit()
