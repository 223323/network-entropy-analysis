fname_1 = 'ent_fs_';
TS_Q_MIN = 0.0;
TS_Q_STEP = 0.05;
TS_Q_MAX = 1.0;
for (q = TS_Q_MIN:TS_Q_STEP:TS_Q_MAX)
%	fname = sprintf('ent_fs_%0.2f.txt', q);
for (ent_typ = 2:1:2)
    if(ent_typ==0)
	fname = sprintf('ent_fs_tsal_%0.2f.txt', q);
	elseif(ent_typ==1)
	fname = sprintf('ent_fs_reny_%0.2f.txt', q);
	elseif(ent_typ==2)
	fname = sprintf('ent_fs_frac_%0.2f.txt', q);
	else
	fname = sprintf('ent_fs_phi_%0.2f.txt', q);
       end


ent_sp = csvread(fname);
attack_start_times = csvread('attack_start_times');

n = length(ent_sp);
t = (0:(n-1))/10;
maxt = n/10;
numAttacks = 15;
sampleAttack = 1;

thplot = [];
tpplot = [];
fpplot = [];
tiplot = [];
mdelay = [];
sdelay = [];

thmin = 0.0;
thmax = 35.0;
timax = 0;

for (th1 = thmin:0.05:thmax)
  [ent_sp_det, ent_sp_filt, ent_sp_filt2] = detect_cusum(ent_sp, th1);
  ent_sp_tp = 0;
  ent_sp_fp = 0;


[s1,s2] = size(ent_sp_det);
% disp(s1);
% disp(s2);
 tp = 0;
 fp = 0;
 ti = 0;
 
 det_flag = zeros(1,numAttacks);	
 tot_delay = 0;
 for (i = 1:numAttacks)
 	a = floor(attack_start_times(i)*10);
   	if(i<numAttacks)
		b = floor(attack_start_times(i+1)*10);
	else
		b = n;
	end
	n1 = a + floor(0.9*(b-a));
	if(i>1)
		for (k = 1:i-1)
			if(det_flag(i-k) == 1)
				attack_started = attack_start_times(i-k+1);
				break;
			end;
		end;
	else
		attack_started = attack_start_times(i);
	end
	for (j = a+1:n1)
		if(ent_sp_det(j)==100)
			tot_delay = tot_delay + 1 + j/10 - a/10;
			ent_sp_tp = ent_sp_tp + 1;
			det_flag(i) = 1;
			sampleAttack = i;
			break;
		end;
	end;
	for (j = n1+1:b-1)
		if(ent_sp_det(j)==100)
			ent_sp_fp = ent_sp_fp + 1;
			break;
		end;
	end;
	
end
if(ent_sp_tp > 0)
 mean_delay = tot_delay / ent_sp_tp;
 else 
 mean_delay = 0;
end


  thplot = [thplot th1];
  tpplot = [tpplot ent_sp_tp];
  fpplot = [fpplot ent_sp_fp];
%  tiplot = [tiplot ti];
  mdelay = [mdelay mean_delay];
  %tpplot = [tpplot ent_pn_tp];
  %fpplot = [fpplot ent_pn_fp];
end

thmin = 0.0;
thmax = 35.0;

for (th1 = thmin:0.05:thmax)
  [ent_sp_det, ent_sp_filt, ent_sp_filt2] = detect_cusum(ent_sp, th1);

 	a = floor(attack_start_times(sampleAttack)*10);
   	if(sampleAttack<numAttacks)
		b = floor(attack_start_times(sampleAttack+1)*10);
	else
		b = n;
	end
	n1 = a + floor(0.9*(b-a));
	attack_started = attack_start_times(sampleAttack);

	sample_delay = 0;
	for (j = a+1:n1)
		if(ent_sp_det(j)==100)
			sample_delay = 1 + j/10 - attack_started;
			break;
		end;
	end;
	sdelay = [sdelay sample_delay];
end
%[ent_pn_tp ent_bn_tp ent_sp_tp ent_dp_tp]
%[ent_pn_fp ent_bn_fp ent_sp_fp ent_dp_fp]

f1 = figure(1);
if (ent_typ==0)
fname = sprintf('thplot_tsal_%0.2f.png', q);
elseif (ent_typ==1)
fname = sprintf('thplot_reny_%0.2f.png', q);
elseif (ent_typ==2)
fname = sprintf('thplot_frac_%0.2f.png', q);
else
fname = sprintf('thplot_phi_%0.2f.png', q);
end
plot(thplot, tpplot/numAttacks, 'b', thplot, fpplot/numAttacks, 'r');
title('CUSUM threshold and detection rate ');
legend('True positive rate', 'False positive rate', 'Location', 'NorthEast');
xlabel('Threshold');
ylabel('Detection rate');
axis([thmin thmax 0 1]);
print(f1, fname, '-dpng');
grid on;

f2 = figure(2);
if (ent_typ==0)
fname = sprintf('tpfpplot_tsal_%0.2f.jpg', q);
elseif (ent_typ==1)
fname = sprintf('tpfpplot_reny_%0.2f.jpg', q);
elseif (ent_typ==2)
fname = sprintf('tpfpplot_frac_%0.2f.jpg', q);
else 
fname = sprintf('tpfpplot_phi_%0.2f.jpg', q);
end
%plot(tpplot/numAttacks, fpplot/numAttacks, 'b-', tpplot/numAttacks, fpplot/numAttacks, 'b*');
plot(tpplot/numAttacks, fpplot/numAttacks, 'b-');
title('Receiver Operating Curve');
xlabel('True positive rate');
ylabel('False positive rate');
axis([0 1 0 1]);
grid on;
print(f2, fname, '-djpg');

f3 = figure(3);
if (ent_typ==0)
fname = sprintf('a_delayplot_fs_tsal_%0.2f.png', q);
elseif (ent_typ==1)
fname = sprintf('a_delayplot_fs_reny_%0.2f.png', q);
elseif (ent_typ==2)
fname = sprintf('a_delayplot_fs_frac_%0.2f.png', q);
else 
fname = sprintf('a_delayplot_fs_phi_%0.2f.png', q);
end
plot(thplot, mdelay, 'b-');
title('average detection delay');
xlabel('threshold');
ylabel('delay');
axis([thmin thmax 0 16]);
grid on;
print(f3, fname, '-dpng');

f4 = figure(4);
if (ent_typ==0)
fname = sprintf('s_delayplot_fs_tsal_%0.2f.eps', q);
elseif (ent_typ==1)
fname = sprintf('s_delayplot_fs_reny_%0.2f.eps', q);
elseif (ent_typ==2)
fname = sprintf('s_delayplot_fs_frac_%0.2f.eps', q);
else 
fname = sprintf('s_delayplot_fs_phi_%0.2f.eps', q);
end
plot(thplot, sdelay, 'b-');
title('sample detection delay');
xlabel('threshold');
ylabel('delay');
axis([thmin thmax 0 16]);
grid on;
print(f4, fname, '-deps');

%f4 = figure(4);
%plot(thplot, tiplot, 'b-');
%title('anomalies detected');
%xlabel('threshold');
%ylabel('detected');
%axis([thmin thmax 0 50]);
%grid on;
%print(f4, 'andetplot', '-djpg');

if (ent_typ==0)
fname = sprintf('adelay_tsal_%0.2f.txt', q);
elseif (ent_typ==1)
fname = sprintf('adelay_reny_%0.2f.txt', q);
elseif (ent_typ==2)
fname = sprintf('adelay_frac_%0.2f.txt', q);
else 
fname = sprintf('adelay_phi_%0.2f.txt', q);
end
csvwrite(fname, mdelay);
if (ent_typ==0)
fname = sprintf('sdelay_tsal_%0.2f.txt', q);
elseif (ent_typ==1)
fname = sprintf('sdelay_reny_%0.2f.txt', q);
elseif (ent_typ==2)
fname = sprintf('sdelay_frac_%0.2f.txt', q);
else 
fname = sprintf('sdelay_phi_%0.2f.txt', q);
end
csvwrite(fname, sdelay);
if (ent_typ==0)
fname = sprintf('tpplot_tsal_%0.2f.txt', q);
elseif (ent_typ==1)
fname = sprintf('tpplot_reny_%0.2f.txt', q);
elseif (ent_typ==2)
fname = sprintf('tpplot_frac_%0.2f.txt', q);
else 
fname = sprintf('tpplot_phi_%0.2f.txt', q);
end
csvwrite(fname, tpplot);
if (ent_typ==0)
fname = sprintf('fpplot_tsal_%0.2f.txt', q);
elseif (ent_typ==1)
fname = sprintf('fpplot_reny_%0.2f.txt', q);
elseif (ent_typ==2)
fname = sprintf('fpplot_frac_%0.2f.txt', q);
else 
fname = sprintf('fpplot_phi_%0.2f.txt', q);
end
csvwrite(fname, fpplot);
fname = sprintf('thplot_%0.2f.txt', q);
csvwrite(fname, thplot);
end;%t
%ddos_plot_cusum
end;%q
