function [satt,eatt] = find_attacks(d,thr)
	s = size(d)(2);
	disp(['size is: ' num2str(s)]);
	
	satt=[]
	eatt=[]
	p = 0;
	wb = false;
	for i = 1:s
		if wb
			if d(i) < thr
				thr = d(i);
				wb = false;
				eatt = [eatt i];
				disp(['eatt: ' num2str(i)])
			end
		else
			if d(i) > thr
				thr = d(i);
				wb = true;
				satt = [satt i];
				disp(['satt: ' num2str(i)])
			end
	end
end
