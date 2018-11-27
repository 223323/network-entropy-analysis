function plot_data(filename, start_time=0.1, end_time=115, subintervals=10)
	data = dlmread(filename);
	
	graphics_toolkit gnuplot
	upto = find(data == 0, 1, 'first');
	% upto = length(data);
	
	% data2 = dlmread('dos_detection.txt');
	detection=0;
	if exist('dos_detection.txt', 'file')
		detection=1;
		data2 = dlmread('dos_detection.txt');
	end
	
	b = max(max(data))+5;
	a = 0;
	
	start_time
	start = max(1, floor(start_time*subintervals))
	upto = end_time*subintervals;
	% upto = max(min(end_time*subintervals, upto), min(length(data), end_time));
	t = linspace(start/subintervals, upto/subintervals, upto-start+1);
	h = figure('visible', false);
	% h = figure('visible', true);
	% axis('tight');
	disp('length is')
	size(data), size(t), upto, size(data(start:upto))
	if length(data) < start
		disp([filename 'not printed'])
		return
	end
	
	% axis('tight')
	plot(t, data(start:upto), '-', "linewidth",2);
	xlabel('t[s]')
	xlim([start_time end_time])
	% ylim('auto')
	
	hold on
	
	if detection == 1
		d2 = data2(1:1:upto);
		A = d2 == 0;
		B = d2 > 0;
	else
		d2 = data(1:1:upto);
		if start_time > 0
			B = t > start_time;
			A = ~B;
		end
	end
	
	if false && start_time > 0
		d2(B) = b;
		d2(A) = a;
		plot(t, d2, '-');
	end
	% set (gca, 'xgrid', 'on')
	% print(h, strcat(filename, '.png'), '-dpng', '-S800,400')
	print(h, strcat(filename, '.png'), '-dpng', '-S400,200')
	% close
end
