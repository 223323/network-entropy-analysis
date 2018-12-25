function plot_data(filename, start_time=0.1, end_time=115, subintervals=10)
	data = dlmread(filename);
	data = [repmat([0], 1, subintervals/2) data]
	graphics_toolkit gnuplot
	end_interval = find(data == 0, 1, 'first');
	% end_interval = length(data);
	
	% data2 = dlmread('dos_detection.txt');
	
	
	
	b = max(max(data))+5;
	a = 0;
	
	% start interval is at least 0.1sec
	start_interval = max(1, floor(start_time*subintervals))
	end_interval = end_time*subintervals;
	span_interval = end_interval-start_interval+1
	t = linspace(start_interval/subintervals, end_interval/subintervals, span_interval);
	h = figure('visible', false);
	% h = figure('visible', true);
	% axis('tight');
	
	disp('length is')
	size(data), size(t), end_interval, size(data(start_interval:end_interval))
	if length(data) < start_interval
		disp([filename 'not printed'])
		return
	end
	
	% axis('tight')
	plot(t, data(start_interval:end_interval), '-', "linewidth",2);
	xlabel('t[s]')
	xlim([start_time end_time])
	% ylim('auto')
	
	hold on
	
	% old dos_detection.txt file
	if exist('data/dos_detection.txt', 'file')
		data2 = dlmread('dos_detection.txt');
		
		d2 = data2(1:1:end_interval);
		A = d2 == 0;
		B = d2 > 0;
		
		if start_time > 0
			d2(B) = b;
			d2(A) = a;
			plot(t, d2, '-');
		end
	end
	
	atk_time_filename = 'data/attack-times.csv';
	if exist(atk_time_filename, 'file')
		t2=csvread(atk_time_filename);
		n=length(t2);
		l=prod(size(t2));
		d=reshape( repmat( reshape(t2, 1, l), 2, 1), 1, l*2 );
		plot(d, repmat([0,b,b,0], 1, n));
	end
	
	% set (gca, 'xgrid', 'on')
	print(h, strcat(filename, '.png'), '-dpng', '-S800,400')
	% print(h, strcat(filename, '.png'), '-dpng', '-S400,200')
	% close
end
