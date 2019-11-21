function plot_all(start_attack_time, end_time, subintervals, attack_times)
	filelist = readdir (pwd);
	for i = 1:numel(filelist)
	  file = filelist{i};
	  [d,n,e] = fileparts(file);
	  skiplist = { '.', '..', 'cmd.txt', 'dos_detection.txt' };
	  # NOTE: strcmp() == 1 if match, 0 if not match
	  if any(strcmp(skiplist, filelist{i})) || !strcmp(e,'.txt')
		disp(['skipping ' filelist{i}])
		continue;
	  end
	  disp(['plotting ' filelist{i}])
	  plot_data(filelist{i}, start_attack_time, end_time, subintervals, attack_times)
	end
end
