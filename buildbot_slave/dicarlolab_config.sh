ADMIN_INFO="Christopher Stawarz <cstawarz@mit.edu>"
MACHINE_DESCRIPTION=$(system_profiler SPHardwareDataType | grep 'Model Identifier:' | cut -d: -f2 | tr -d ' ')
