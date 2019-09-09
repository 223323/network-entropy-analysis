function ddos_cusum2_i3_delay(entropy_file, subintervals, attack_times, sgn=1)
	graphics_toolkit gnuplot
	
	entropy = csvread(entropy_file);
	attack_start_times = csvread(attack_times);

	n = length(entropy);
	t = (0:(n-1))/subintervals;
	maxt = n/subintervals;

	attack_start_times = attack_start_times(1,:)
	numAttacks = size(attack_start_times)(2);
	disp(['numAttacks: ' num2str(numAttacks)])
	sampleAttack = 1;

	true_positive_ratio = 0.5;

	thplot = []; % threshold
	tpplot = []; % true positive
	fpplot = []; % false positive

	mdelay = []; % mean delay
	sdelay = []; % sample delay

	thmin = 0.0;
	thmax = 35.0;
	thinc = 1.5;
	
	% cusums = [];
	for th1 = thmin:thinc:thmax % iterate threshold range
		disp(['th ' num2str(th1)]);
		
		[entropy_detection, entropy_filt, entropy_filt2] = detect_cusum(entropy, th1, sgn);
		% cusums = [cusums entropy_detection];
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
			
			b1 = a + floor(true_positive_ratio*(b-a)); % (a |=======|--- b)
			
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

		disp(['tp,fp: ' num2str(entropy_tp_count) ', ' num2str(entropy_fp_count)]);
	end

	thmin = 0.0;
	thmax = 35.0;

	% th2 = 1;
	for th1 = thmin:thinc:thmax
		% th2 = th2 + 1;
		disp(['th ' num2str(th1)]);
		[entropy_detection, entropy_filt, entropy_filt2] = detect_cusum(entropy, th1, sgn);
		% entropy_detection = cusums(th2, :);

		a = floor(attack_start_times(sampleAttack) * subintervals);
		if sampleAttack < numAttacks
			b = floor(attack_start_times(sampleAttack+1)*subintervals);
		else
			b = n;
		end
		
		b1 = a + floor(true_positive_ratio*(b-a));
		attack_started = attack_start_times(sampleAttack);
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
	csvwrite('data/tpplot.txt', tpplot);
	csvwrite('data/fpplot.txt', fpplot);
	csvwrite('data/mdelay.txt', mdelay);
	csvwrite('data/sdelay.txt', sdelay);


end
