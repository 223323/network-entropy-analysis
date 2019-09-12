import subprocess
import os, sys
import shutil
import datetime

files = [
	# filename, length[s]
	('lan-big-1', 500),
	# ('lan-big-2', 500),
	# ('lan-big-3', 500),
	# ('lan-big-4', 500),
]

pcap_dir = 'pcap/'

entropies = [
	# entropy, Q-range
	# ('bhatiasingh', (0.5,15,0.1)),
	('bhatiasingh', (3.3,15,0.1)),
	# ('bhatiasingh', (5,15,0.1)),
	# ('ubriaco', (0,1,0.1)),
	# ('tsalis', (-2,2,0.1)),
	# ('renyi', (-2,2,0.1)),
]

def frange(x, y, jump):
  while x <= y:
    yield x
    x += jump
    
for ff in files:
	f, end_time = ff
	pcap_file = os.path.join(pcap_dir, f+'.cap')
	attack_times_file = os.path.join(pcap_dir, f+'.csv')
	for ent in entropies:
		entropy, ent_range = ent
	# for entropy in ['tsalis']:
		for q in frange(*ent_range):
			if q == 0.0: continue
						
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
			print('running ', cmdline)
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
				# '--path', os.path.abspath('scripts'),
				'--path', '../scripts',
				'--eval', 'plot_all({start_attack}, {end_time}, {subintervals}, "{attack_times}")'.format(**oct_args),
			]
			subprocess.Popen(['bash', '-c', 'echo $PWD'])
			print('running: ', cmdline)
			process = subprocess.Popen(cmdline, cwd='output')
			output, error = process.communicate()
			############################
			
			entropy_dir_name = 'entropy'
			
			subprocess.Popen(['bash', '-c', 'mkdir {entropy}; mv *.png {entropy}; mv *.txt {entropy}'.format(entropy=entropy_dir_name)], cwd='output')
			
			# do cusum
			if True:
				for cus in ['ent_sp','ent_fsd']:
					os.mkdir(os.path.join('output','data'))
					
					# cusum
					############################
					oct_args = {
						'entropy_file': os.path.join(entropy_dir_name, cus+'.txt'),
						'subintervals': 10,
						'attack_times': os.path.join('..',attack_times_file),
						# 'sgn': 0 if cus == 'ent_stream' else 1
						'sgn': 1
					}
					cmdline = ['octave-cli',
						'--path', '../scripts',
						'--eval', 'ddos_cusum2_i3_delay("{entropy_file}", {subintervals}, "{attack_times}", {sgn})'.format(**oct_args),
					]
					
					process = subprocess.Popen(cmdline, cwd='output')
					output, error = process.communicate()
					############################
					
					shutil.move('output/data', 'output/'+'cusum_'+cus)
			
			
			
			## Save
			today = datetime.datetime.today()
			outdir = today.strftime(f+' --'+entropy + ' -Q ' + ('%.2f' % q) + ' (%d.%m %H:%M:%S)')
			shutil.move('output', os.path.join('outputs', outdir))
