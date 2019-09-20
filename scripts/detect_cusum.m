function [detection, d, h] = detect_cusum(samples, M, sgn=1, adp_k=false)

	% detection
	%	100 - detected
	%	0 - not detected
	% d - difference
	% 	value [0, ...)
	% h - 
	
	function r = interpolate(alpha, a, b)
		r = alpha * a + (1-alpha) * b;
	end

	alfa = 0.2;
	alfa2 = 0.05;
	K = 0.3;

	if adp_k
		K = (max(samples) + min(samples)) / 20
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

% a = 0.2, b = 0.05, M = K = 0.3
% bool cusum(double *samples, int n, double a, double b, double M, double K) {
	% double d,s,h;
	% for(int i=0; i < n; i++) {
		% double sample = samples[i];
		% new_sample = old_sample * (1-a) + sample * a;
		% old_sample = new_sample;
		% d = max(0, d + new_sample - old_sample - K);
		% s = s * (1-b) + pow(new_sample - old_sample, 2) * b;
		% h = M * sqrt(s)

		% if(d > h) {
			% return true;
		% }
	% }
% }
