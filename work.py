import subprocess
import os, sys
import shutil
import datetime

files = [
	# filename, length[s]
	# ('lan-big-1', 500),
	# ('lan-big-4', 500),
	# ('lan-big-10', 500),
	# ('lan-big-7', 500),
	
	# ('lan-big-5-nf', 500),
	# ('lan-big-5-rf', 500),
	
	# ('lan-big-2', 500),
	# ('lan-big-3', 500),
	# ('lan-big-4', 500),
	
	
	# ('lan-big-10-1_10', 500),
	# ('lan-big-10-1_20', 500),
	
	# ('lan-big-10-1', 500),
	
	# zavrseno
	# ('lan-big-10-1_1', 500),
	
	# ('lan-big-10-1_5', 500),
	# ('lan-big-10-1_40', 500),
	# ('lan-big-10-1_20', 500),
	# ('lan-big-10-1_80', 500),
	# ('lan-big-10-1_160', 500),
	# ('lan-big-10-1_320', 500),
	('lan-big-10-1_240', 500),
	# ('my-largescale.ns2', 500),
]

pcap_dir = 'pcap/'

def frange(x, y, jump, *args):
  while x <= y:
    yield x
    x += jump

entropies = [
	# entropy, Q-range
	# ('bhatiasingh', (0.5,15,0.1)),
	# ('bhatiasingh', (3.0,15,1.0)),
	# ('bhatiasingh', (5,15,0.1)),
	# ('tsalis', (-2,2,0.1)),
	# ('tsalis', (1,2,0.1)),
	# ('renyi', (-2,2,0.1)),
	# ('renyi2', (0,2,0.1)),
	
	# ('bhatiasingh', (3.3,15,0.1)),
	
	
	('shannon', (0,0,1)),
	('bhatiasingh', (0.5,15,0.1)),
	('ubriaco', (0,1,0.1)),
	('tsalis2', (-2,2,0.1)),
	('renyi', (-2,2,0.1)),
	
	# ('renyi', (1,2,0.1)),
	
	# ('renyi', [0,0.1,0.2]),
]

cusums = [
	'ent_sp',
	'ent_fsd',
	# 'ent_bn',
	# 'ent_pn',
]

r=subprocess.Popen(['make'])
r.wait()
if r.returncode != 0: exit(1)

os.system('rm -rf output')
for ff in files:
	f, end_time = ff
	
	today = datetime.datetime.today()
	date = today.strftime(' (%d.%m-%H:%M)')
	# date = ''
	
	pcap_dir2 = os.path.join(pcap_dir, f)
	pcap_file = os.path.join(pcap_dir2, f+'.cap' if f.find('.') == -1 else f)
	attack_times_file = os.path.join(pcap_dir2, f+'.csv')
	
	for ent in entropies:
		entropy, ent_range = ent
		for q in (frange(*ent_range) if type(ent_range) is tuple else ent_range):
			if os.path.exists('output'):
				shutil.rmtree('output')
			
			# entropy
			############################
			cmdline = ['./entropy',
				pcap_file,
				'--end-time', str(end_time),
				'--'+entropy,
				'--entropy-q', ('%.2f' % q),
				'--no-verbose',
				# '--fsd1',
			]
			print('running ', ' '.join(cmdline))
			process = subprocess.Popen(cmdline)
			output, error = process.communicate()
			############################
			

			# octave plots
			############################
			oct_args = {
				'start_attack': 0.01,
				'end_time': end_time,
				'subintervals': 10,
				'attack_times': os.path.join('..',attack_times_file)
			}
			
			cmdline = ['octave-cli',
				'--path', '../scripts',
				'--eval', 'plot_all({start_attack}, {end_time}, {subintervals}, "{attack_times}")'.format(**oct_args),
			]
			subprocess.Popen(['bash', '-c', 'echo $PWD'])
			print('running: ', ' '.join(cmdline))
			process = subprocess.Popen(cmdline, cwd='output')
			output, error = process.communicate()
			############################
			
			entropy_dir_name = 'entropy'
			
			subprocess.Popen(['bash', '-c', 'mkdir {entropy}; mv *.png {entropy}; mv *.txt {entropy}'.format(entropy=entropy_dir_name)], cwd='output')
			
			# CUSUM
			for cus in cusums:
				os.mkdir(os.path.join('output','data'))
				
				# cusum
				############################
				oct_args = {
					'entropy_file': os.path.join(entropy_dir_name, cus+'.txt'),
					'subintervals': 10,
					'attack_times': os.path.join('..',attack_times_file),
					# 'sgn': 0 if cus == 'ent_stream' else 1
					'sgn': '1',
					# 'adp': 'true' if entropy.startswith('tsalis') else 'false'
					'adp': 'true'
				}
				cmdline = ['octave-cli',
					'--path', '../scripts',
					'--eval', 'ddos_cusum2_i3_delay("{entropy_file}", {subintervals}, "{attack_times}", {sgn}, {adp})'.format(**oct_args),
				]
				
				print('running: ', ' '.join(cmdline))
				process = subprocess.Popen(cmdline, cwd='output')
				output, error = process.communicate()
				############################
				
				shutil.move('output/data', 'output/'+'cusum_'+cus)
			

			## Save
			
			# main_outdir / entropy_dir / outdir
			outdir = f+' --'+entropy + ' -Q ' + ('%.2f' % q)
			
			main_outdir = os.path.join('outputs', f+date)
			entropy_dir = os.path.join(main_outdir, entropy)
			
			if not os.path.exists(main_outdir):
				os.mkdir(main_outdir)
			if not os.path.exists(entropy_dir):
				os.mkdir(entropy_dir)
			
			shutil.move('output', os.path.join(entropy_dir, outdir))
			
