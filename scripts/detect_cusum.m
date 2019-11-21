function [detection, d, h] = detect_cusum(samples, M, sgn=1, adp_k=false)

	% detection
	%	100 - detected
	%	0 - not detected
	% d - difference
	% h - root mean square
	
	function r = interpolate(alpha, a, b)
		r = alpha * a + (1-alpha) * b;
	end

	alfa = 0.2;
	alfa2 = 0.05;
	K = 0.3;

	if adp_k
		K = (max(samples) + min(samples)) / 50
	end
	
	n = length(samples);

	d          = zeros(1, n);
	h          = zeros(1, n);
	detection  = zeros(1, n);

	mi_a = samples(1);
	s = 3;
	prev_d = 0;

	for j = 1:n % j = over all samples
		sample = samples(j);
		
		mi_a = interpolate(alfa, sample, mi_a);
		
		prev_d = max(0, prev_d + sample - (mi_a + K) );
		d(j) = prev_d;
		
		s = interpolate(alfa2, (sample - mi_a)^2, s);
		h(j) = M*sqrt(s);
	
		if d(j) > h(j)
			detection(j) = 100;
		end
	end
end
