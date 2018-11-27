function ddos_cusum2_i3_delay(ent, att)
	%ent_pn = csvread('ent_pn.txt');
	%ent_bn = csvread('ent_bn.txt');
	graphics_toolkit gnuplot
	% ent_sp = csvread('ent_sp.txt');
	ent_sp = csvread(ent);

	%ent_dp = csvread('ent_dp.txt');
	%tot_syn = csvread('tot_syn.txt');

	% attack_start_times = csvread('attack_start_times.txt');
	attack_start_times = csvread(att);

	%tot_syn = csvread('tot_pn.txt');

	subintervals = 10;

	n = length(ent_sp);
	t = (0:(n-1))/subintervals;
	maxt = n/subintervals;

	numAttacks = size(attack_start_times)(2);
	disp(['numAttacks: ' num2str(numAttacks)])
	sampleAttack = 1;

	true_positive_ratio = 0.5;

	thplot = [];
	tpplot = [];
	fpplot = [];

	% tiplot = [];
	mdelay = [];
	sdelay = [];

	thmin = 0.0;
	thmax = 35.0;
	timax = 0;

	thinc = 0.5;



	for (th1 = thmin:thinc:thmax) % iterate threshold range

		%[ent_pn_det, ent_pn_filt, ent_pn_filt2] = detect_cusum(100-ent_pn, th1);
		%[ent_bn_det, ent_bn_filt, ent_bn_filt2] = detect_cusum(100-ent_bn, th1);
		[ent_sp_detection, ent_sp_filt, ent_sp_filt2] = detect_cusum(ent_sp, th1);
		%[ent_dp_det, ent_dp_filt, ent_dp_filt2] = detect_cusum(ent_dp, th1);
		%[ent_dp_det, ent_dp_filt, ent_dp_filt2] = detect_cusum2(tot_syn, th1);
		disp(['th ' num2str(th1)]);

		% how much sp true positives
		ent_sp_tp_count = 0;
		
		% how much sp false positives
		ent_sp_fp_count = 0;

		[s1,s2] = size(ent_sp_detection);
		% disp(s1);
		% disp(s2);
		tp = 0;
		fp = 0;
		% ti = 0;

		det_flag = zeros(1,numAttacks);	
		tot_delay = 0;
		
		for i = 1:numAttacks
		
			% pick a, b of given interval from file
			a = floor(attack_start_times(i)*subintervals);
			if(i < numAttacks)
				b = floor(attack_start_times(i+1)*subintervals);
			else
				b = n;
			end
			
			n1 = a + floor(true_positive_ratio*(b-a)); % (a |=======|--- b)
			
			disp(['interval: ' num2str(a) ' ' num2str(b)])
			
			% find when did attack start
			% if i > 1
				% for k = 1:i-1
				
					% if det_flag(i-k) == 1
						% attack_started = attack_start_times(i-k+1);
						% break;
					% end;
					
				% end;
			% else
				% attack_started = attack_start_times(1);
			% end
			
			
			% check for attacks in true positive interval
			for j = a+1:n1 % j = (a |=======|--- b)
			
				if(ent_sp_detection(j) == 100)
					%tot_delay = tot_delay + 1 + j/10 - attack_started;
					% tot_delay = tot_delay + 1 + (j - a) / subintervals;
					tot_delay = tot_delay + (j - a) / subintervals;
					ent_sp_tp_count = ent_sp_tp_count + 1;
					det_flag(i) = 1;
					sampleAttack = i;
					break;
				end;
				
			end;
			
			% how much false positives (check from false positive interval)
			for j = n1+1:b-1 % j = (a --------|===| b)
			
				if(ent_sp_detection(j)==100)
					ent_sp_fp_count = ent_sp_fp_count + 1;
					break;
				end;
				
			end;
			
		end
		
		
		% mean delay calc
		if ent_sp_tp_count > 0
			mean_delay = tot_delay / ent_sp_tp_count;
		else 
			mean_delay = 0;
		end

		% append resulting arrays
		thplot = [thplot th1];
		
		% total true positive attacks
		tpplot = [tpplot ent_sp_tp_count];
		
		% total false positive attacks
		fpplot = [fpplot ent_sp_fp_count];
		
		%  tiplot = [tiplot ti];
		
		% mean delays
		mdelay = [mdelay mean_delay];
		%tpplot = [tpplot ent_pn_tp];
		%fpplot = [fpplot ent_pn_fp];
		
		% result per threshold is:
			% (threshold, total_true_positive_attacks, total_true_positive_attacks, mean_delay)
		disp(['tp,fp: ' num2str(ent_sp_tp_count) ', ' num2str(ent_sp_fp_count)]);
	end

	thmin = 0.0;
	thmax = 35.0;

	for th1 = thmin:thinc:thmax
		%[ent_pn_det, ent_pn_filt, ent_pn_filt2] = detect_cusum(100-ent_pn, th1);
		%[ent_bn_det, ent_bn_filt, ent_bn_filt2] = detect_cusum(100-ent_bn, th1);
		[ent_sp_detection, ent_sp_filt, ent_sp_filt2] = detect_cusum(ent_sp, th1);
		%[ent_dp_det, ent_dp_filt, ent_dp_filt2] = detect_cusum(ent_dp, th1);
		%[ent_dp_det, ent_dp_filt, ent_dp_filt2] = detect_cusum2(tot_syn, th1);

		a = floor(attack_start_times(sampleAttack) * subintervals);
		if sampleAttack < numAttacks
			b = floor(attack_start_times(sampleAttack+1)*subintervals);
		else
			b = n;
		end
		
		n1 = a + floor(true_positive_ratio*(b-a));
		attack_started = attack_start_times(sampleAttack);
		sample_delay = 0;
		
		for j = a+1:n1
		
			if ent_sp_detection(j) == 100
				% sample_delay = 1 + j/subintervals - attack_started;
				sample_delay = j/subintervals - attack_started;
				break;
			end;
			
		end;
		
		sdelay = [sdelay sample_delay];
	end

	% [ent_pn_tp ent_bn_tp ent_sp_tp_count ent_dp_tp]
	% [ent_pn_fp ent_bn_fp ent_sp_fp_count ent_dp_fp]

	% true positive rate, false positive rate
	f1 = figure(1, 'visible', false);
	plot(thplot, tpplot/numAttacks, 'b-', 'linewidth', 2);
	hold on
	plot(thplot, fpplot/numAttacks, 'r-', 'linewidth', 2);
	plot(thplot, mdelay, 'c-', 'linewidth', 2);

	title('CUSUM (SYN packets)');
	legend('True positive rate', 'False positive rate', 'Location', 'NorthWest');
	xlabel('Threshold');
	ylabel('Detection rate');
	axis([thmin thmax, 0 2]);
	print(f1, 'thplot', '-djpg');
	grid on;

	% 
	if false
		f2 = figure(2, 'visible', false);
		% plot(tpplot/numAttacks, fpplot/numAttacks, 'b-', tpplot/numAttacks, fpplot/numAttacks, 'b*');
		plot(tpplot/numAttacks, fpplot/numAttacks, 'b*');
		title('CUSUM (SYN packets)');
		xlabel('True positive rate');
		ylabel('False positive rate');
		axis([0 1, 0 1]);
		grid on;
		print(f2, 'tpfpplot', '-djpg');
	end
	hold off

	% threshold, delay
	% -----------------
	f3 = figure(3, 'visible', false);
	plot(thplot, mdelay, 'b-');
	title('average detection delay');
	xlabel('threshold');
	ylabel('delay');
	axis([thmin thmax 0 8]);
	grid on;
	print(f3, 'a_delayplot', '-djpg');

	% sample attack
	% ----------------
	% threshold, delay
	f4 = figure(4, 'visible', false);
	plot(thplot, sdelay, 'b-');
	title('sample detection delay');
	xlabel('threshold');
	ylabel('delay');
	axis([thmin thmax 0 8]);
	grid on;
	print(f4, 's_delayplot', '-djpg');

	%f4 = figure(4);
	%plot(thplot, tiplot, 'b-');
	%title('anomalies detected');
	%xlabel('threshold');
	%ylabel('detected');
	%axis([thmin thmax 0 50]);
	%grid on;
	%print(f4, 'andetplot', '-djpg');

	# write csv-s
	csvwrite('tpplot.txt', tpplot);
	csvwrite('fpplot.txt', fpplot);
	% csvwrite('tiplot.txt', tiplot);



	% plot(rand(1, 10));       % Plot some random data
	% ylabel(gca, 'scale 1');  % Add a label to the left y axis
	% set(gca, 'Box', 'off');  % Turn off the box surrounding the whole axes
	% axesPosition = get(gca, 'Position');           % Get the current axes position
	% hNewAxes = axes('Position', axesPosition, ...  % Place a new axes on top...
					% 'Color', 'none', ...           %   ... with no background color
					% 'YLim', [0 10], ...            %   ... and a different scale
					% 'YAxisLocation', 'right', ...  %   ... located on the right
					% 'XTick', [], ...               %   ... with no x tick marks
					% 'Box', 'off');                 %   ... and no surrounding box
	% ylabel(hNewAxes, 'scale 2');  % Add a label to the right y axis

end
