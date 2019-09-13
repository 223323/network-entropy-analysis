function plot_data(filename, start_time=0.1, end_time=115, subintervals=10, attack_times="data/attack-times.csv")
	data = dlmread(filename);
	data = [repmat([0], 1, subintervals/2) data];
	graphics_toolkit gnuplot
	end_interval = find(data == 0, 1, 'first');
	% end_interval = length(data);
	% data2 = dlmread('dos_detection.txt');
	
	data(data == Inf) = 0;
	b = max(max(data));
	a = 0;
	
	% start interval is at least 0.1sec
	start_interval = max(1, floor(start_time*subintervals));
	end_interval = end_time*subintervals;
	span_interval = end_interval-start_interval+1;
	t = linspace(start_interval/subintervals, end_interval/subintervals, span_interval);
	h = figure('visible', false);
	% h = figure('visible', true);
	% axis('tight');
	
	% disp('length is')
	% size(data), size(t), end_interval, size(data(start_interval:end_interval))
	if length(data) < start_interval
		disp([filename 'not printed'])
		return
	end
	plot_data = data(start_interval:end_interval);
	ll = size(t);
	ll1 = size(plot_data);
	% axis('tight')
	plot(t, plot_data, '-', "linewidth",2);
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
	
	atk_time_filename = attack_times;
	% attack_times
	if exist(atk_time_filename, 'file')
		t2=csvread(atk_time_filename);
		% disp(['attack times opened'])
		
		n=length(t2);
		l=prod(size(t2));
		d=reshape( repmat( reshape(t2, 1, l), 2, 1), 1, l*2 );
		d = d - 0.5;
		
		plot_data = repmat([0,b,b,0], 1, n);
		% ll2 = size(d)
		% ll3 = size(plot_data)
		% d,plot_data
		plot(d, plot_data, 'r-');
	end
	
	subintervals
	
	% set (gca, 'xgrid', 'on')
	print(h, strcat(filename, '.png'), '-dpng', '-S1600,400')
	% print(h, strcat(filename, '.png'), '-dpng', '-S400,200')
	% close
end
