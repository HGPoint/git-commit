local log = require("hg.log")

local M = {}

local COMMIT_INFO_PATH = "gitcommit/commitinfo"
local REVISION_LENGTH = 10

local function read_commit_hash()
	local file, open_error = io.open(COMMIT_INFO_PATH, "r")

	if not file then
		log.debug(
			string.format(
				"WARNING: Failed to open commit information file: %s (%s)",
				COMMIT_INFO_PATH,
				open_error or "unknown error"
			)
		)
		return nil
	end

	local revision = file:read("*l")
	file:close()

	if not revision or revision == "" then
		log.debug("WARNING: Commit information file is empty or unreadable: " .. COMMIT_INFO_PATH)
		return nil
	end

	return revision:sub(1, REVISION_LENGTH)
end

function M.apply_revision_override()
	local revision = read_commit_hash()

	if not revision then
		log.debug("INFO: project.revision override will not be applied.")
		return
	end

	log.debug("INFO: Detected commit hash: " .. revision .. ". Overriding sys.get_config.")

	local original_get_config = sys.get_config

	sys.get_config = function(key, default_value)
		if key == "project.revision" then
			return revision
		end
		return original_get_config(key, default_value)
	end

	log.debug(
		"INFO: sys.get_config successfully overridden. project.revision is now: " .. sys.get_config("project.revision")
	)
end

return M
