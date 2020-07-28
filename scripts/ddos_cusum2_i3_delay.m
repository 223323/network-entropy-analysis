function ddos_cusum2_i3_delay(entropy_file, subintervals, attack_times_file, sgn=1, adp=false)
	
	graphics_toolkit gnuplot
	entropy = csvread(entropy_file);
	attack_times = csvread(attack_times_file);

	n = length(entropy);
	t = (0:(n-1))/subintervals;
	maxt = n/subintervals;

	attack_start_times = attack_times(1,:)
	attack_end_times = attack_times(2,:)
	
	numAttacks = size(attack_start_times)(2);
	
	sampleAttack = 1;

	true_positive_ratio = 0.5;

	thplot = []; % threshold
	tpplot = []; % true positive
	fpplot = []; % false positive

	mdelay = []; % mean delay
	sdelay = []; % sample delay

	% thmin = 5.0;
	thmin = 0.0;
	thmax = 35.0;
	thinc = 1.0;
	
	[entropy_detection, entropy_filt, entropy_filt2] = detect_cusum(entropy, 1, 1, adp); % speedup
	
	progress = false;
	
	disp(['numAttacks: ' num2str(numAttacks)]);
	for th1 = thmin:thinc:thmax % iterate threshold range
		if progress
			disp(['th ' num2str(th1)]);
		end
		
		% [entropy_detection, entropy_filt, entropy_filt2] = detect_cusum(entropy, th1, sgn);
		entropy_detection = (entropy_filt > th1*entropy_filt2) * 100; % speedup
		
		% ent_det = max(entropy_detection);
		######### DEBUG ############
		if false
			hold off
			f1 = figure(2, 'visible', false);
			legend('detection', 'diff', 'sqrt(diff^2)', 'attack', 'Location', 'NorthWest');
			b = max(entropy_filt + entropy_filt2);
			
			plot(1:n, entropy_detection * b/100*1.05, 'g-')
			hold on
			plot(1:n, entropy_filt, 'b-')
			plot(1:n, entropy_filt2, 'c-')
			
			# attack times
			t2=csvread(attack_times_file)*subintervals;
			% t2=attack_times
			n2=length(t2);
			l=prod(size(t2));
			d = reshape( repmat( reshape(t2, 1, l), 2, 1 ), 1, l*2 );
			% d = d - 0.5;
			plot_data = repmat([0,b,b,0], 1, n2);
			plot(d, plot_data, 'r-');
			
			mkdir('data/dbg')
			print(f1, sprintf('data/dbg/ent-%.2f.jpg', th1), '-djpg', '-S1600,400');
		end
		#########################
		
		% how much sp true positives
		entropy_tp_count = 0;
		
		% how much sp false positives
		entropy_fp_count = 0;

		[s1,s2] = size(entropy_detection);
		tp = 0;
		fp = 0;

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
			
			b = min(n, b);
			% b1 = min(n, a + floor(true_positive_ratio*(b-a))); % (a |=======|--- b)
			b1 = floor(attack_end_times(i)*subintervals);
			% disp(['interval: ' num2str(a) ' ' num2str(b)])
			
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
			for j = a+1:b1 % j = (a |=======|--- b)
			
				if(entropy_detection(j) == 100)
					tot_delay = tot_delay + (j - a) / subintervals;
					entropy_tp_count = entropy_tp_count + 1;
					det_flag(i) = 1;
					sampleAttack = i;
					break;
				end;
				
			end;
			
			% how much false positives (check from false positive interval)
			for j = b1+1:b-1 % j = (a --------|===| b)
			
				if(entropy_detection(j)==100)
					entropy_fp_count = entropy_fp_count + 1;
					break;
				end;
				
			end;
			
		end
		
		
		% mean delay calc
		if entropy_tp_count > 0
			mean_delay = tot_delay / entropy_tp_count;
		else 
			mean_delay = 0;
		end

		% append resulting arrays
		thplot = [thplot th1];
		
		% total true positive attacks
		tpplot = [tpplot entropy_tp_count];
		
		% total false positive attacks
		fpplot = [fpplot entropy_fp_count];
		
		% mean delays
		mdelay = [mdelay mean_delay];

		if progress
			disp(['tp,fp: ' num2str(entropy_tp_count) ', ' num2str(entropy_fp_count) ', ' num2str(numAttacks)]);
		end
	end

	% process sample
	for th1 = thmin:thinc:thmax
		if progress
			disp(['th ' num2str(th1)]);
		end
		% [entropy_detection, entropy_filt, entropy_filt2] = detect_cusum(entropy, th1, sgn);
		entropy_detection = (entropy_filt > th1*entropy_filt2) * 100; % speedup

		a = floor(attack_start_times(sampleAttack) * subintervals);
		if sampleAttack < numAttacks
			b = floor(attack_start_times(sampleAttack+1)*subintervals);
		else
			b = n;
		end
		
		% b1 = a + floor(true_positive_ratio*(b-a));
		b1 = floor(attack_end_times(sampleAttack))*subintervals;
		attack_started = attack_start_times(sampleAttack)*subintervals;
		sample_delay = 0;
		
		for j = a+1:b1
		
			if entropy_detection(j) == 100
				sample_delay = 1 + j/subintervals - attack_started;
				% sample_delay = j/subintervals - attack_started;
				break;
			end;
			
		end;
		
		sdelay = [sdelay sample_delay];
	end

	graphics_toolkit gnuplot
	% true positive rate, false positive rate
	% -----------------
	f1 = figure(1, 'visible', false);
	plot(thplot, tpplot/numAttacks, 'b-', 'linewidth', 2);
	hold on
	plot(thplot, fpplot/numAttacks, 'r-', 'linewidth', 2);
	plot(thplot, mdelay, 'c-', 'linewidth', 2);

	title('CUSUM (SYN packets)');
	legend('True positive rate', 'False positive rate', 'Average Delay', 'Location', 'NorthWest');
	xlabel('Threshold');
	ylabel('Detection rate');
	axis([thmin thmax, 0 2]);
	grid on;
	print(f1, 'data/thplot', '-djpg');
	hold off;
	
	% f2 = figure(2,'visible',false);
	
	% print(f2, 'data/cusum_det', '-djpg');

	% threshold, average delay
	% -----------------
	f3 = figure(3, 'visible', false);
	plot(thplot, mdelay, 'b-');
	hold on
	plot(thplot, sdelay, 'c-');
	legend('Average Delay', 'Sample Delay', 'Location', 'NorthWest');
	title('detection delay');
	xlabel('threshold');
	ylabel('delay');
	axis([thmin thmax 0 8]);
	grid on;
	print(f3, 'data/delay', '-djpg');


	# write csv-s
	% csvwrite('data/tpplot.txt', tpplot);
	% csvwrite('data/fpplot.txt', fpplot);
	% csvwrite('data/mdelay.txt', mdelay);
	% csvwrite('data/sdelay.txt', sdelay);


end
