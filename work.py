import subprocess
import os, sys
import shutil
import datetime

files = [
	# filename, length[s]
	# ('lan-big-1', 500),
	('lan-big-4', 500),
	
	# ('lan-big-5-nf', 500),
	# ('lan-big-5-rf', 500),
	
	# ('lan-big-2', 500),
	# ('lan-big-3', 500),
	# ('lan-big-4', 500),
]

pcap_dir = 'pcap/'

entropies = [
	# entropy, Q-range
	# ('bhatiasingh', (0.5,15,0.1)),
	# ('bhatiasingh', (3.3,15,0.1)),
	# ('bhatiasingh', (5,15,0.1)),
	# ('ubriaco', (0,1,0.1)),
	('tsalis', (-2,2,0.1)),
	# ('tsalis2', (-2,2,0.1)),
	# ('renyi', (-2,2,0.1)),
	# ('renyi2', (0,2,0.1)),
	# ('renyi', (0,2,0.1)),
	# ('shannon', (0,0,1))
]

cusums = [
	'ent_sp',
	'ent_fsd',
	# 'ent_bn',
	# 'ent_pn',
]


subprocess.Popen(['make']).wait()

def frange(x, y, jump, *args):
  while x <= y:
    yield x
    x += jump
    
for ff in files:
	f, end_time = ff
	pcap_file = os.path.join(pcap_dir, f+'.cap')
	attack_times_file = os.path.join(pcap_dir, f+'.csv')
	for ent in entropies:
		entropy, ent_range = ent
		for q in frange(*ent_range):
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
					'adp': 'true' if entropy.startswith('tsalis') else 'false'
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
			today = datetime.datetime.today()
			outdir = today.strftime(f+' --'+entropy + ' -Q ' + ('%.2f' % q) + ' (%d.%m %H:%M:%S)')
			shutil.move('output', os.path.join('outputs', outdir))
