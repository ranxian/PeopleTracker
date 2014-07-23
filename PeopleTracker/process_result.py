from __future__ import division
import xml.etree.ElementTree as ET
import sys
from collections import defaultdict

class Processor:
	def __init__(self, input_path, output_path):
		self.output_path = output_path
		self.tree = ET.parse(input_path)
		self.root = self.tree.getroot()
		self.frame_set = {}
		self.object_set = {}
		self.kernel_size = 10

	def process(self):
		self.create_database()
		self.denoise()
		self.smooth_boxsize()
		self.smooth_trackpos()
		new_root = self.formtree()
		new_root.write(self.output_path)

	def create_database(self):
		self.nframe = 0

		for frame in self.root.iter('frame'):
			self.nframe += 1
			frame_number = int(frame.get('number'))
			for obj in frame.iter('object'):
				obj_id = int(obj.get('id'))
				box = [b for b in obj.iter('box')][0]
				if self.object_set.get(obj_id) == None:
					self.object_set[obj_id] = {}
				self.object_set[obj_id][frame_number] = box.attrib
	
	def calc_mean(self, numbers):
		to_calc = [number for number in numbers if number != None]
		total = reduce(lambda x, y: x + y, to_calc)
		return float(total) / len(to_calc)

	def calc_var(self, numbers):
		to_calc = [number for number in numbers if number != None]
		if len(to_calc) == 1:
			return 0
		mean = self.calc_mean(to_calc)
		total = reduce(lambda x, y: x + (y-mean)*(y-mean), to_calc) 
		return total / (len(to_calc)-1)

	def denoise(self):
		frame_counts = map(lambda key: len(self.object_set[key]), self.object_set.keys())
		mean = self.calc_mean(frame_counts)
		var = self.calc_var(frame_counts)

		# Delete object with very few frames
		ndelete = 0
		for obj_id in self.object_set.keys():
			if len(self.object_set[obj_id]) < mean / 4:
				del self.object_set[obj_id]
				ndelete += 1
		print ndelete, 'deleted'

	def smooth_boxsize(self):
		EMPTY_BOX = {'w': 0, 'h': 0, 'xc': 0, 'yc': 0}

		# Smooth
		for obj_id in self.object_set.keys():
			obj = self.object_set[obj_id]
			boxes = []
			result = []
			boxes = [obj.get(i+1) for i in range(self.kernel_size)]
			for i in range(self.nframe):
				if not obj.get(i+1) == None:
					cur_box = obj.get(i+1)
					w_m = 0
					h_m = 0
					n = 0
					for box in boxes:
						if box == None:
							continue
						else:
							w_m += float(box['w'])
							h_m += float(box['h'])
							n += 1
					if n > 1:
						w_m /= n
						h_m /= n
						result.append({'xc': cur_box['xc'], 'yc': cur_box['yc'], 'w': str(w_m), 'h': str(h_m)})
					else:
						result.append(cur_box)
				else:
					result.append(None)
				del boxes[0]
				boxes.append(obj.get(i+1+self.kernel_size))
			obj = {}
			for idx, box in enumerate(result):
				if box != None:
					obj[idx+1] = box
			self.object_set[obj_id] = obj

	def smooth_trackpos(self):
		for obj_id in self.object_set.keys():
			obj = self.object_set[obj_id]
			boxes = []
			result = []
			boxes = [obj.get(i+1) for i in range(self.kernel_size)]
			for i in range(self.nframe):
				if not obj.get(i+1) == None:
					cur_box = obj.get(i+1)
					xc_m = 0
					yc_m = 0
					n = 0
					for box in boxes:
						if box == None:
							continue
						else:
							xc_m += float(box['xc'])
							yc_m += float(box['yc'])
							n += 1
					if n > 1:
						xc_m /= n
						yc_m /= n
						result.append({'xc': str(xc_m), 'yc': str(yc_m), 'w': cur_box['w'], 'h': cur_box['h']})
					else:
						result.append(cur_box)
				else:
					result.append(None)
				del boxes[0]
				boxes.append(obj.get(i+1+self.kernel_size))
			obj = {}
			for idx, box in enumerate(result):
				if box != None:
					obj[idx+1] = box
			self.object_set[obj_id] = obj

	def objset_to_frmset(self):
		for obj_id in self.object_set.keys():
			obj = self.object_set[obj_id]
			for frame in obj.keys():
				if self.frame_set.get(frame) == None:
					self.frame_set[frame] = {}
				self.frame_set[frame][obj_id] = obj[frame]

	def formtree(self):
		self.objset_to_frmset()
		new_root = ET.Element('dataset')
		for frame in self.frame_set.keys():
			frame_doc = ET.SubElement(new_root, 'frame')
			frame_doc.set('number', str(frame))
			objlist_doc = ET.SubElement(frame_doc, 'objectlist')
			for obj_id in self.frame_set[frame].keys():
				box = self.frame_set[frame][obj_id]
				obj_doc = ET.SubElement(objlist_doc, 'object')
				obj_doc.set('confidence', '1')
				obj_doc.set('id', str(obj_id))
				box_doc = ET.SubElement(obj_doc, 'box')
				box_doc.set('yc', box['yc'])
				box_doc.set('xc', box['xc'])
				box_doc.set('w', box['w'])
				box_doc.set('h', box['h'])
		return ET.ElementTree(new_root)


if len(sys.argv) < 2:
	print 'usage: python precess_result.py <input_xml> [output_xml]'
else:
	input_path = sys.argv[1]
	# input_path = 'Data\\clip-result.xml'
	output_path = 'result-processed.xml'
	if len(sys.argv) > 2:
		output_path = sys.argv[2]
	processor = Processor(input_path, output_path)
	processor.process()
