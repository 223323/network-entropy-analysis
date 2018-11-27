function plot_all(start_attack_time, end_time, subintervals)
	filelist = readdir (pwd);
	for i = 1:numel(filelist)
	  file = filelist{i};
	  [d,n,e] = fileparts(file);
	  skiplist = { 'cmd.txt', 'dos_detection.txt' };
	  if regexp (filelist{i}, "^\\.\\.?$") || strcmp(skiplist, filelist{i}) || !strcmp(e,'.txt') || strcmp('cmd.txt', filelist{i})
		disp(['skipping ' filelist{i}])
		continue;
	  end
	  disp(['plotting ' filelist{i}])
	  plot_data(filelist{i}, start_attack_time, end_time, subintervals)
	end
end
