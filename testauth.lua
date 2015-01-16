function plugin_init(opts)
  print("plugin init")
  for key, value in pairs(opts) do
    print(key, value)
  end
end

function plugin_cleanup(opts)
  print("plugin cleanup")
end

function security_init(opts, reload)
  print("security init")
end

function security_cleanup(opts, reload)
  print("security cleanup")
end

function acl_check(clientid, username, topic, access)
  print("acl check")
  return true
end

function unpwd_check(username, password)
  print("unpwd check")
  return true
end
