#pragma once

enum event_type {
	event_ping, event_get_rc6, event_login, event_setup_cortana, event_load_cortana, event_mac_addresses, event_terminate_client,
	event_debugger_attached, event_timing_attack, event_anti_debugger_detect, event_sem_wait_timeout, event_debug_object, event_no_debug_inherit, event_rasterise_change, event_illegal_dx_hook, event_illegal_dxcreate_hook,
	event_wrong_checksum, event_halo_checksum, event_strings_checksum, event_loader_checksum, event_map_checksum,
	event_join_client, event_join_server, event_leave_server, event_screenshot, event_process, event_module, event_report, event_alias
};