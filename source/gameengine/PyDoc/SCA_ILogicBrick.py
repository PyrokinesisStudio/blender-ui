# $Id$
# Documentation for the logic brick base class SCA_ILogicBrick
from KX_GameObject import *

class SCA_ILogicBrick:
	"""
	Base class for all logic bricks.
	"""
	
	def getOwner():
		"""
		Gets the game object associated with this logic brick.
		
		@rtype: L{KX_GameObject}
		"""
	def setExecutePriority(priority):
		"""
		Sets the priority of this logic brick.
		
		@type priority: integer
		@param priority: the priority of this logic brick.
		"""
	def getExecutePriority():
		"""
		Gets the execution priority of this logic brick.
		
		@rtype: integer
		@return: this logic bricks current priority.
		"""
