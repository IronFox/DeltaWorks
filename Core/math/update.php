<?php

	/*
		Hungarian notations used in this file:
		
		rg...			Range/Array of type ...
		ar...			Range/Array of type ...
		hm...			Hashmap (from string to ...)
		i...			Ingeger
		n...			Unsigned integer
		o...			Object (not further specified)
		b...			Boolean
		null_...		Possibly undefined (NULL) in addition to its other possible values
		str...			String
		ref...			Reference (e.g. refstrExample (= reference to string named 'example') or refrgExample (= reference to range named 'example'))
		h...			
			hfile...	File handle
		
		
		special:
		ar_MyClassInstances[_...]	Array of objects of type MyClassInstance
		hm_MyClassInstances[_...]	Hash map to objects of type MyClassInstance
		MyClassInstance[_...]		Single object of type MyClassInstance
		ref_MyClassInstance[_...]	Reference to a single oject of type
	*/

	$ar_CFunctionDefinitions = array();			//All functions. Entries are all of type CFunctionDefinition
	$hmiFunctionIndexTable = array();	//Map of function keys to the index of the respective entry in $ar_CFunctionDefinitions

	$iTemporaryVariableIndex = 0;		//!< Current index to be used for new temporary variables
	$iHelperClassIndex = 0;				//!< Current index to be used for new helper class implementations
	$iGlobalIndent = 0;					//!< Global indentation modifier. Lines will be indented more or less according to this value
	$bUseDimensionConstant = false;		//Indicates that a compile-time-constant 'Dimensions' is defined and should be used to determine the length of vectors/arrays
	$bUseConstReferences = false;		//Indicates that const references should be used to pass scalar values (e.g. const double&), rather than copies (e.g. double). Those versions should be used for large/dynamic types
	$bIterationTail = false;
	$bIterationHead = false;
	$hfileErr = NULL;
	$hmbDeclaredVectorVariables = array();		//!< All vector variables that have been declared to this point of the function implementation. Value is always true.
	
	$ar_CTemplateLoops = array();		//!< All helper classes needed
	$hmiHelperIndexTable = array();				//!< Map of template helper class key to the respective entry in $ar_CTemplateLoops
	
	$strStatic = '';
//	$strVectorBaseClass = 'VectorV_Include__';
	
	
	$bImplementZeroDimensionUnrollHelperClasses = true;	//!< Print unroll helper classes for 0 iterations as well. These classes will not do anything.
	
	$bDataIsPrimary = true;	//!< Indicates that the data currently imported should be exported. If this variable is set false then read data is used for internal reference but not exported
	
	
	$strBeforeFunctionType = "inline";
	$strBeforeFunctionName = "";
	$strBeforeFunctionBody = "throw()";
	$strVector = 'TVector';
	$strGlobalNamespace = 'Vector';
	
	
	
	class ScanMode
	{
		const Idle = 'Idle';					//Base mode
		const InConfig = 'InConfig';				//Parsing configuration lines
		const InFunctions = 'InFunctions';			//Expecting function definitions
		const InFunctionComment = 'inFunctionComment';		//Expecting function comment, if any, function body
		const InFunctionBlockComment = 'inFunctionBlockComment';		//Reading subsequent comment lines
		const InFunctionLines = 'InFunctionLines';		//Expecting function body
	}
	
	
	class AssemblyMode
	{
		const Objects = 'Objects';			//Use objects to reference vectors (e.g. TVector3<...> or TVector<...,D>)
		const Pointers = 'Pointers';		//Use raw pointers to reference vectors (e.g. const float v[3])
		const DynamicPointers = 'DynamicPointers';		//Use raw pointers of dynamic length (e.g. const float*v). 'dimensions' should be referenced to determine current dimensions
	}
	
	$enAssemblyMode = AssemblyMode::Objects;
	
	function classBase($null_strNamespace=NULL)
	{
		if (!$null_strNamespace)
		{
			global $strGlobalNamespace;
			$null_strNamespace = $strGlobalNamespace;
		}
		return $null_strNamespace.'V_Include__';
	}
	
	function functionBase()
	{
		global $strGlobalNamespace;
		return $strGlobalNamespace;
	}
	
	/**
		@brief Prints a message to the stderr stream
		
		@param strMsg Message to print
	*/
	function err($strMsg)
	{
		global $hfileErr;
		if (!$hfileErr)
			$hfileErr = fopen("php://stderr","w+");
		fputs($hfileErr,$strMsg."\n");
	}
	
	/**
		@brief Terminates script execution with the specified error message
		
		The error message will be written to both standard and stderr output
		
		@param strMsg String message to display when terminating
	*/
	function fatal($strMsg="Unspecified error")
	{
		err($strMsg);
		die("Fatal exception occurred here. Message was: \"$strMsg\"");
	}
	
	
	$bImplementationStarted = false;
	
	/**
		@brief Indicates that a new function implementation is attempted
		
		Resets any global status variables affected during previous function implementations
	*/
	function beginFunctionImplementation()
	{
		global $bImplementationStarted;
		if ($bImplementationStarted)
			fatal("Already implementing function");
		$bImplementationStarted = true;
		ob_start();
	}
	
	function beginFunctionVarianceImplementation()
	{
		global $hmbDeclaredVectorVariables;
		$hmbDeclaredVectorVariables = array();
	}
	
	/**
		@brief Concludes function implementation and replaces float types
		
		@param $strFloatType Type to replace :float with
	*/
	function endFunctionImplementation($strFloatType)
	{
		global $bImplementationStarted;
		if (!$bImplementationStarted)
			fatal("Not currently implementing function");
		$bImplementationStarted = false;
		
			$implementation = ob_get_contents();
		ob_end_clean();
		echo str_replace(":float",$strFloatType,$implementation);
	}
	
	function makeFunctionKey($strName, $nParameters)
	{
		return "$strName:$nParameters";
	}
	
	/**
		@brief Compiles an object vector type, using the specified float type, dimensions, and distinction whether or not the context uses the specified number of dimensions, even if in dimension-constant-mode
		
		@param $strScalarType		Floating point type to use
		@param $nDimensions			Number of dimensions applicable in the current context. This value is ignored and replaced with 'Dimensions' if the global variable $bUseDimensionConstant is true and the parameter $bFixedDimensions is false
		@param $bFixedDimensions	Declares that the specified number of dimensions should be used even if the global variable $bUseDimensionConstant is set true
		@return vector type string
	*/
	function vtype($strScalarType,$nDimensions,$bFixedDimensions)
	{
		global $bUseDimensionConstant,$strVector;
		
		if ($bUseDimensionConstant && !$bFixedDimensions)
			return "$strVector<$strScalarType,Dimensions>";
		if ($nDimensions <= 4)
			return "$strVector$nDimensions<$strScalarType>";
				
		return "$strVector<$strScalarType,$nDimensions>";
	}
	
	function	scalarType($ar_CFunctionParameters)
	{
		$index = 0;
		foreach ($ar_CFunctionParameters as $p)
		{
			if (!$p->bIsConst && ($p->bIsVector || $p->bIsReference))
				return templateTypeName($index);
			$index++;
		}
		return templateTypeName(0);
	}
	
	function	convertObjectToString(&$oEntry, $iIndex)
	{
		$oEntry = $oEntry->toString();
	}

	function objectsToStrings($aroField)
	{
		array_walk($aroField, 'convertObjectToString');
		return $aroField;
	}
	
	function reduceObjectToParameter(&$oEntry, $iIndex, $strParameterName)
	{
		$oEntry = $oEntry->$strParameterName;
	}
	
	function reduceObjectsToParameterValue(&$aroField, $strParameterName)
	{
		array_walk($aroField, 'reduceObjectToParameter', $strParameterName);
	}
	
	function implode_p($strGlue, $aroField, $strParameter)
	{
		$rs = array();
		foreach($aroField as $entry)
			array_push($rs,$entry->$strParameter);
		return implode($strGlue, $rs);
	}
	

	
	function implode_pv($strGlue, $ar_CFunctionParameters)
	{
		global $enAssemblyMode;
		$out = array();
		foreach ($ar_CFunctionParameters as $f)
		{
			$bIsVector = $f->bIsVector;
			$s = $f->strName;
			if ($bIsVector && $enAssemblyMode == AssemblyMode::Objects)
				$s .=".v";
			array_push($out,$s);
		}	
		return implode($strGlue,$out);
	}
	
	function unifiedParameterName($index)
	{
		return "__p$index";
	}
	
	function unifiedParameters($nCount)
	{
		$rs = array();
		for ($i = 0; $i < $nCount; $i++)
			array_push($rs,unifiedParameterName($i));
		return implode(',', $rs);
	}	
	
	function templateTypeName($iIndex)
	{
		return "T$iIndex";
	}
	
	function templateType($iIndex)
	{
		return 'typename '.templateTypeName($iIndex);
	}
	
	function declareTemplates($nParameters)
	{
		if (!$nParameters)
			fatal("Trying to declare 0 template types");
		$strDeclaration = 'template <'.templateType(0);
		for ($i = 1; $i < $nParameters; $i++)
			$strDeclaration .= ", ".templateType($i);
		
		return $strDeclaration.'>';
	}
	
	function adjustVectorExpressions($strLine)
	{
		global $enAssemblyMode;
		
		switch ($enAssemblyMode)
		{
			case AssemblyMode::Objects:
				return preg_replace("/:([0-9]+|Current)/",'.v[${1}]',str_replace(array(":0",":1",":2",":3"),array(".x",".y",".z",".w"),preg_replace("/:([0-9][0-9]+)/",'.v[${1}]',$strLine)));
			case AssemblyMode::Pointers:
			{
			//	echo "<in: '$strLine'>\n";
				$rs = preg_replace("/:([0-9]+|Current)/","[\\1]",$strLine);
			//	echo "<out: '$rs'>\n;";
				return $rs;
			}
			case AssemblyMode::DynamicPointers:
				return preg_replace("/:([0-9]+|iterator__)/","[\\1]",$strLine);
		}
		fatal("Invalid assembly mode '$enAssemblyMode' for operation adjustVectorExpressions($strLine)");
	}
	
	
	define ("RegOpChar","[\)=+\-*\/%&|\'\"~^]");
	define ("RegIdChars",'a-zA-Z0-9_');
	define ("RegIdChar",'['.RegIdChars.']');
	
	function pointerAdjustVectorIteratorExpressions($strLine)
	{
		return adjustVectorExpressions(preg_replace("/:i([^".RegIdChars.']|$)/',":Current\\1",$strLine));
	}
	
	function pointerAdjustDynamicVectorIteratorExpressions($strLine,$strIterator)
	{
		return adjustVectorExpressions(preg_replace("/:i([^".RegIdChars.']|$)/',":$strIterator\\1",$strLine));
	}
	
	
	function makeLineUniform($l, $ar_CFunctionParameters)
	{
		$p_index = 0;
		foreach ($ar_CFunctionParameters as $p)
		{
			$expression = '/(^|[^a-zA-Z_0-9])'.$p->strName.'($|[^a-zA-Z_0-9.])/';
			$replace_with = '${1}'.unifiedParameterName($p_index++).'${2}';
			
			$l->content = preg_replace($expression,$replace_with,$l->content);
			$l->content = preg_replace($expression,$replace_with,$l->content);//may overlap
			//echo "leads to '$l->content'\n";
			
			$l->content = preg_replace('/('.RegOpChar.')\s+('.RegIdChar.')/','${1} ${2}',$l->content);
			$l->content = preg_replace('/('.RegOpChar.')\s+('.RegIdChar.')/','${1} ${2}',$l->content);
			
			$l->content = preg_replace('/('.RegIdChar.')\s+('.RegOpChar.')/','${1} ${2}',$l->content);
			$l->content = preg_replace('/('.RegIdChar.')\s+('.RegOpChar.')/','${1} ${2}',$l->content);
			
			$l->content = preg_replace('/\s\s+/', ' ', $l->content);
		}
		return $l->content;
	}
	
	function makeUniform(&$ar_CLines, $ar_CFunctionParameters)
	{
		$p_index = 0;
		foreach ($ar_CFunctionParameters as $p)
		{
			$expression = '/(^|[^a-zA-Z_0-9])'.$p->strName.'($|[^a-zA-Z_0-9.])/';
			$replace_with = '${1}'.unifiedParameterName($p_index++).'${2}';
			
			foreach ($ar_CLines as &$l)
			{
				//echo "replacing '$expression' with '$replace_with' in '$l->content' ";
				$l->content = preg_replace($expression,$replace_with,$l->content);
				$l->content = preg_replace($expression,$replace_with,$l->content);//may overlap
				//echo "leads to '$l->content'\n";
				
				$l->content = preg_replace('/('.RegOpChar.')\s+('.RegIdChar.')/','${1} ${2}',$l->content);
				$l->content = preg_replace('/('.RegOpChar.')\s+('.RegIdChar.')/','${1} ${2}',$l->content);
				
				$l->content = preg_replace('/('.RegIdChar.')\s+('.RegOpChar.')/','${1} ${2}',$l->content);
				$l->content = preg_replace('/('.RegIdChar.')\s+('.RegOpChar.')/','${1} ${2}',$l->content);
				
				$l->content = preg_replace('/\s\s+/', ' ', $l->content);
			}
		}
	}
	
	/**
		@brief Registers a required template unroll helper class
		
		@param $strFunctionName Name of the function that is requesting this helper. Only the first requesting function will be applied
		@param $ar_CFunctionParameters Parameters that the helper class should accept
		@param $ar_CLines Lines that the helper class should implement. Alternative this parameter may be a single string
		@param $iDimensionsReduction (constant) Number of iterations that should be skipped at the end. This number should only ever be 0 or 1
		@param $null_strReturnType Optional specific return type. Set NULL to let the function determine the return type automatically
		@return class name method instantiation similar to "BaseClass<...>::User_". Parenthesis will not be included
	*/
	function requestIterationClass($enHelperType, $strTypeParameter, $strFunctionName, $ar_CFunctionParameters,$ar_CLines, $iDimensionReduction, $null_strReturnType=NULL)
	{
		if (gettype($ar_CLines) == "string")
		{
			$line = new CLine();
			$line->content = $ar_CLines;
			$ar_CLines = array($line);
		}
		global $ar_CTemplateLoops;
		global $hmiHelperIndexTable;
		global $enAssemblyMode;
		global $iHelperClassIndex;
		
		$keys = array();
		foreach ($ar_CFunctionParameters as $p)
			$keys[] = trim($p->classify());
		$keys[] = "<>";
		foreach ($ar_CLines as $CLine)
			$keys[] = $CLine->content;
		
		$key = $enAssemblyMode."/$enHelperType:".$strTypeParameter."||".implode("|",$keys);
		
		if (array_key_exists($key,$hmiHelperIndexTable))
		{
			$ar_CTemplateLoops[$hmiHelperIndexTable[$key]]->bClass = true;
			if (!strlen($ar_CTemplateLoops[$hmiHelperIndexTable[$key]]->strClassBase))
				$ar_CTemplateLoops[$hmiHelperIndexTable[$key]]->strClassBase = classBase();
			return $ar_CTemplateLoops[$hmiHelperIndexTable[$key]]->strClassBase."<$iDimensionReduction,Dimensions,false>::".$ar_CTemplateLoops[$hmiHelperIndexTable[$key]]->strName;
		}
		
		global $bDataIsPrimary;
		
		
		
		$CTemplateLoop = new CTemplateLoop();
		$CTemplateLoop->ar_CFunctionParameters = $ar_CFunctionParameters;
		$CTemplateLoop->ar_CLines= $ar_CLines;
		$CTemplateLoop->strKey = $key;
		$CTemplateLoop->enAssemblyMode = $enAssemblyMode;
		$CTemplateLoop->strName = $enHelperType.($iHelperClassIndex++)."_$strFunctionName";
		$CTemplateLoop->enType = $enHelperType;
		$CTemplateLoop->strTypeParameter = $strTypeParameter;
		$CTemplateLoop->strReturnType = $null_strReturnType?$null_strReturnType:($enHelperType != CTemplateLoop::LogicOperator?scalarType($ar_CFunctionParameters):"bool");
		$CTemplateLoop->bClass = true;
		$CTemplateLoop->bExport = $bDataIsPrimary;
		$CTemplateLoop->strClassBase = classBase();
		
		
		
		$hmiHelperIndexTable[$key] = count($ar_CTemplateLoops);
		$ar_CTemplateLoops[] = $CTemplateLoop;
		
		return $CTemplateLoop->strClassBase."<$iDimensionReduction,Dimensions,false>::".$CTemplateLoop->strName;		
	}

	
	/**
		@brief Registers a required template loop function
		
		@param $strFunctionName Name of the function that is requesting this helper. Only the first requesting function will be applied
		@param $ar_CFunctionParameters Parameters that the helper class should accept
		@param $ar_CLines Lines that the helper class should implement. Alternative this parameter may be a single string
		@param $iDimensionsReduction (constant) Number of iterations that should be skipped at the end. This number should only ever be 0 or 1
		@param $null_strReturnType Optional specific return type. Set NULL to let the function determine the return type automatically
		@return class name method instantiation similar to "BaseClass<...>::User_". Parenthesis will not be included
	*/
	function requestDynamicIterationFunction($enHelperType, $strTypeParameter, $strFunctionName, $ar_CFunctionParameters,$ar_CLines, $iDimensionReduction, $null_strReturnType=NULL)
	{
		if (gettype($ar_CLines) == "string")
		{
			$line = new CLine();
			$line->content = $ar_CLines;
			$ar_CLines = array($line);
		}
		global $ar_CTemplateLoops;
		global $hmiHelperIndexTable;
		global $enAssemblyMode;
		global $iHelperClassIndex;
		
		$keys = array();
		foreach ($ar_CFunctionParameters as $p)
			$keys[] = trim($p->classify());
		$keys[] = "<>";
		foreach ($ar_CLines as $CLine)
			$keys[] = $CLine->content;
		
		$key = $enAssemblyMode."/$enHelperType:".$strTypeParameter."||".implode("|",$keys);
		
		if (array_key_exists($key,$hmiHelperIndexTable))
		{
			$ar_CTemplateLoops[$hmiHelperIndexTable[$key]]->bFunction = true;
			if (!strlen($ar_CTemplateLoops[$hmiHelperIndexTable[$key]]->strFunctionBase))
				$ar_CTemplateLoops[$hmiHelperIndexTable[$key]]->strFunctionBase = functionBase();
			
			return $ar_CTemplateLoops[$hmiHelperIndexTable[$key]]->strFunctionBase.'::'.$ar_CTemplateLoops[$hmiHelperIndexTable[$key]]->strName.'D';
		}
		
		global $bDataIsPrimary;
		
		
		
		$CTemplateLoop = new CTemplateLoop();
		$CTemplateLoop->ar_CFunctionParameters = $ar_CFunctionParameters;
		$CTemplateLoop->ar_CLines= $ar_CLines;
		$CTemplateLoop->strKey = $key;
		$CTemplateLoop->enAssemblyMode = $enAssemblyMode;
		$CTemplateLoop->strName = $enHelperType.($iHelperClassIndex++)."_$strFunctionName";
		$CTemplateLoop->enType = $enHelperType;
		$CTemplateLoop->strTypeParameter = $strTypeParameter;
		$CTemplateLoop->strReturnType = $null_strReturnType?$null_strReturnType:($enHelperType != CTemplateLoop::LogicOperator?scalarType($ar_CFunctionParameters):"bool");
		$CTemplateLoop->bFunction = true;
		$CTemplateLoop->bExport = $bDataIsPrimary;
		$CTemplateLoop->strFunctionBase = functionBase();
		
		
		
		$hmiHelperIndexTable[$key] = count($ar_CTemplateLoops);
		$ar_CTemplateLoops[] = $CTemplateLoop;
		
		return $CTemplateLoop->strFunctionBase.'::'.$CTemplateLoop->strName.'D';
	}

	

	function indent($iDepth)
	{
		global $iGlobalIndent;
		$rs = "";
		for ($i = 0; $i < $iGlobalIndent+$iDepth; $i++)
			$rs.="\t";
		return $rs;
	}
	
	function beginsWith($strHaystack, $strBegin)
	{
		return substr($strHaystack,0,strlen($strBegin))==$strBegin;
	}
	
	function eraseSegmentIfBeginsWith(&$strHaystack, $strBegin)
	{
		if (beginsWith($strHaystack,$strBegin))
		{
			$strHaystack = substr($strHaystack,strlen($strBegin));
			return true;
		}
		return false;
	}
	
	function parseParameters($strParameterSequence, &$arstrParameters, $iInitialDepth = 1, &$iClosingParenthesisIndex=NULL)
	{
		$start = 0;
		$level = $iInitialDepth;
		$bString = false;
		for ($i = 0; $i < strlen($strParameterSequence); $i++)
		{
			$chr = $strParameterSequence[$i];
			switch ($chr)
			{
				case '\'':
				case '"':
					$bString = !$bString;
				break;
				case '{':
				case '(':
				case '[':
					if (!$bString)
						$level++;
				break;
				case ')':
				case '}':
				case ']':
					if (!$bString)
					{
						$level--;
						if (!$level)
						{
							if ($start < $i)
							{
								$strParameter = trim(substr($strParameterSequence,$start,$i-$start));
								if (strlen($strParameter) > 0)
									$arstrParameters[] = $strParameter;
							}
							if (isset($iClosingParenthesisIndex))
								$iClosingParenthesisIndex = $i;
							return 0;
						}
					}
				break;
				case ',':
					if ($level == 1 && !$bString)
					{
						$strParameter = trim(substr($strParameterSequence,$start,$i-$start));
						if (strlen($strParameter) > 0)
							$arstrParameters[] = $strParameter;
						$start = $i+1;
					}
				break;
			}
		}
		if ($start < strlen($strParameterSequence))
		{
			$strParameter = trim(substr($strParameterSequence,$start));
			if (strlen($strParameter) > 0)
				$arstrParameters[] = $strParameter;
		}
		if (isset($iClosingParenthesisIndex))
			$iClosingParenthesisIndex = -1;
		return $level;
	}
	
	
	function	recPutBinary($pattern, $func, $begin, $end)
	{
		if ($end - $begin <= 1)
			return str_replace(":i",":$begin", $pattern);
		
		$div = (int)(($end+$begin)/2);
		
		return $func."(".recPutBinary($pattern,$func,$begin,$div).", ".recPutBinary($pattern,$func,$div,$end).")";
	}
	
	function	putSimple($pattern,$op,$d)
	{
		$rs =  str_replace(":i",":0",$pattern);
		for ($i = 1; $i < $d; $i++)
		{
			$rs .= " $op ".str_replace(":i",":$i",$pattern);
		}
		return $rs;
	}			
	
	function replaceFunction(&$line, $d,$CFunctionDefinition, $current_indent, $extra_indent)
	{
		$segments = preg_split ( "/([^:]|^)(:)(max|min|and|or|sum|glue|makeVector|vector|<[a-zA-Z_][a-zA-Z_0-9]*>)[ \t]*\(/" , $line,-1,PREG_SPLIT_DELIM_CAPTURE);
		
		global $iTemporaryVariableIndex;
		global $bUseDimensionConstant;
		global $enAssemblyMode;
		/*
		ob_start();
			print_r($segments);
			err(ob_get_contents());
		ob_end_clean();
			*/
		
		if (count($segments) < 3)
			return false;
		//$line = "";
		$bChanged = false;
		$out = array();
		for ($i = 0; $i < count($segments); $i++)
		{
			if (!strlen($segments[$i]))
				continue;
			if ($segments[$i] == ':')
			{
				$next = -1;
				$iDepth = 1;
				$parameters = array();
				while ($iDepth > 0 && $i+1 < count($segments))
				{
					$i++;
					$iDepth = parseParameters($segments[$i+1],$parameters,$iDepth,$next);
				}
				//echo "/* ".$segments[$i+1]." => ";
				//print_r($parameters);
				//echo "*/\n";
				
				if ($segments[$i][0] == '<')
				{
					$func_name = trim($segments[$i],"<>:");
					//echo "user function: '".$func_name."'\n";
					
					global $ar_CFunctionDefinitions;
					global $hmiFunctionIndexTable;
					
					foreach ($parameters as &$p)
					{
						if (preg_match("/[\(\[{,.\-+*\/%&|?:;]/",$p))	//some of these are technically not possible, but not taking chances here
							$p = "($p)";
					}
					
					$key = makeFunctionKey($func_name, count($parameters));
					if (isset($hmiFunctionIndexTable[$key]))
					{
						$func = $ar_CFunctionDefinitions[$hmiFunctionIndexTable[$key]];
						
						if (count($func->lines) == 1)
						{
							ob_start();
								//echo "/*".$func->return_type." / ".$func->toString()."*/";
								$func->implementInnerBody($d,"",$parameters,$extra_indent,true);
								$out[] = "(".ob_get_contents().")";
							ob_end_clean();
							$bChanged = true;
						}
						else
						{
							if ($func->bMultipleReturns)
							{
								global $strGlobalNamespace;
								if ($strGlobalNamespace != $func->strNamespace)
									$call = $func->strNamespace.'::';
								else
									$call = '';
								$call .= $func->names[0];
								if ($enAssemblyMode == AssemblyMode::Pointers)
								{
									$call .= $d;
									$out[] = $call . "(".implode(",",$parameters).")";
								}
								else
									if ($enAssemblyMode == AssemblyMode::DynamicPointers)
									{
										$call .= "D";
										$out[] = $call . "(".implode(",",$parameters).",dimensions)";
									}
									else
										$out[] = $call . "(".implode(",",$parameters).")";
								$bChanged = true;
							}
							else
							{
								$rtype = $func->returnType();
								$var = $rtype!='void'?"temp__".($iTemporaryVariableIndex++):"";
								$previous = array_pop($out);
								ob_start();
									echo "\n";
									echo indent($current_indent+$extra_indent)."//block inlining ".$func->toString()." for dimensions=$d, assembly_mode='$enAssemblyMode', parameters={".implode(", ",$parameters)."}...\n";
									if ($var)
										echo indent($current_indent+$extra_indent)."$rtype $var;\n";
									echo indent($current_indent+$extra_indent)."{\n";
										$func->implementInnerBody($d,$var?"$var=":false,$parameters,$extra_indent+$current_indent-1);
									echo indent($current_indent+$extra_indent)."}";
									$out[] = ob_get_contents();
								ob_end_clean();
								$out[] = $previous;
								$out[] = $var;
								$bChanged = true;
							}
						}
					}
					else
					{
						echo "functions registered in function-map:\n";
						print_r($hmiFunctionIndexTable);
						fatal("Unable to find function declaration '$func_name' using ".count($parameters)." parameter(s) {".implode(", ",$parameters)."}");
					}
				}
				else
				{
					$func_name = $segments[$i];
					if (!count($parameters))
					{
						fatal("All template functions require at least one parameter. Error occurred while trying to parse template function :$func_name()");
					}
					
					
					
					switch ($func_name)
					{
						case 'max':
							if ($enAssemblyMode == AssemblyMode::DynamicPointers)
								$out[] = requestDynamicIterationFunction(CTemplateLoop::Functional,"vmax","max",$CFunctionDefinition->parameters,$parameters[0],0)."(".implode_pv(", ",$CFunctionDefinition->parameters).", dimensions)/*$parameters[0]*/";
							else if ($bUseDimensionConstant)
								$out[] = requestIterationClass(CTemplateLoop::Functional,"vmax","max",$CFunctionDefinition->parameters,$parameters[0],0)."(".implode_pv(", ",$CFunctionDefinition->parameters).")/*$parameters[0]*/";
							else
								$out[] = recPutBinary($parameters[0],"vmax",0,$d);
							$bChanged = true;
						break;
						case 'min':
							if ($enAssemblyMode == AssemblyMode::DynamicPointers)
								$out[] = requestDynamicIterationFunction(CTemplateLoop::Functional,"vmin","min",$CFunctionDefinition->parameters,$parameters[0],0)."(".implode_pv(", ",$CFunctionDefinition->parameters).", dimensions)/*$parameters[0]*/";
							else if ($bUseDimensionConstant)
								$out[] = requestIterationClass(CTemplateLoop::Functional,"vmin","min",$CFunctionDefinition->parameters,$parameters[0],0)."(".implode_pv(", ",$CFunctionDefinition->parameters).")/*$parameters[0]*/";
							else
								$out[] = recPutBinary($parameters[0],"vmin",0,$d);
							$bChanged = true;
						break;
						case "and":
							if ($enAssemblyMode == AssemblyMode::DynamicPointers)
								$out[] = requestDynamicIterationFunction(CTemplateLoop::LogicOperator,"&&","and",$CFunctionDefinition->parameters,$parameters[0],0)."(".implode_pv(", ",$CFunctionDefinition->parameters).", dimensions)/*$parameters[0]*/";
							else if ($bUseDimensionConstant)
								$out[] = requestIterationClass(CTemplateLoop::LogicOperator,"&&","and",$CFunctionDefinition->parameters,$parameters[0],0)."(".implode_pv(", ",$CFunctionDefinition->parameters).")/*$parameters[0]*/";
							else
								$out[] = putSimple($parameters[0],"&&",$d);
							$bChanged = true;
						break;
						case "or":
							if ($enAssemblyMode == AssemblyMode::DynamicPointers)
								$out[] = requestDynamicIterationFunction(CTemplateLoop::LogicOperator,"||","or",$CFunctionDefinition->parameters,$parameters[0],0)."(".implode_pv(", ",$CFunctionDefinition->parameters).", dimensions)/*$parameters[0]*/";
							else if ($bUseDimensionConstant)
								$out[] = requestIterationClass(CTemplateLoop::LogicOperator,"||","or",$CFunctionDefinition->parameters,$parameters[0],0)."(".implode_pv(", ",$CFunctionDefinition->parameters).")/*$parameters[0]*/";
							else
								$out[] = putSimple($parameters[0],"||",$d);
							$bChanged = true;
						break;
						case "sum":
							if ($enAssemblyMode == AssemblyMode::DynamicPointers)
								$out[] = requestDynamicIterationFunction(CTemplateLoop::Operator,"+","sum",$CFunctionDefinition->parameters,$parameters[0],0)."(".implode_pv(", ",$CFunctionDefinition->parameters).", dimensions)/*$parameters[0]*/";
							else if ($bUseDimensionConstant)
								$out[] = requestIterationClass(CTemplateLoop::Operator,"+","sum",$CFunctionDefinition->parameters,$parameters[0],0)."(".implode_pv(", ",$CFunctionDefinition->parameters).")/*$parameters[0]*/";
							else
								$out[] = putSimple($parameters[0],"+",$d);
							$bChanged = true;
						break;
						case "glue":
							if (count($parameters) < 2)
								fatal("Glue template function requires two parameters: glue(glue, glued_item). Error while trying to parse template function :glue(".$parameters[0].")");
							if ($enAssemblyMode == AssemblyMode::DynamicPointers)
								$out[] = requestDynamicIterationFunction(CTemplateLoop::Operator,"+".$parameters[0]."+","glue",$CFunctionDefinition->parameters,$parameters[1],0,$CFunctionDefinition->return_type)."(".implode_pv(", ",$CFunctionDefinition->parameters).", dimensions)/*$parameters[0]*/";
							else if ($bUseDimensionConstant)
								$out[] = requestIterationClass(CTemplateLoop::Operator,"+".$parameters[0]."+","glue",$CFunctionDefinition->parameters,$parameters[1],0,$CFunctionDefinition->return_type)."(".implode_pv(", ",$CFunctionDefinition->parameters).")/*$parameters[0]*/";
							else
								$out[] = putSimple($parameters[1],"+".$parameters[0]."+",$d);
							$bChanged = true;
						break;
						case 'vector':
						case 'makeVector':
							switch ($enAssemblyMode)
							{
								case AssemblyMode::Objects:
									$out[] = vtype(':float',$d,false)." ".implode(", ",$parameters);
								break;
								case AssemblyMode::Pointers:
									$out[] = ':float '.implode("[".($bUseDimensionConstant?"Dimensions":$d)."], ",$parameters)."[".($bUseDimensionConstant?"Dimensions":$d)."]";
								break;
								case AssemblyMode::DynamicPointers:
									$out[] = "Array<:float> ".implode("(dimensions), ",$parameters)."(dimensions)";
								break;
							}
							global $hmbDeclaredVectorVariables;
							foreach ($parameters as $p)
								$hmbDeclaredVectorVariables[$p] = true;
							$bChanged = true;
						break;
						case "float":
							$out[] = "(:float)(".implode(", ",$parameters).")";
							$bChanged = true;
						break;
						default:
							if ($func_name == "")
							{
								$next = -1;
							}
							else
								fatal("Trying to implement unknown template function ':$func_name(".$parameters[0].")'");
					}
				}
				$out[] = substr($segments[$i+1],$next+1);
				$i++;
			}
			else
				$out[] = $segments[$i];
		}
		$line = implode("",$out);
		//echo $line."\n";
		//die("end");
		return $bChanged;
			/*
		
		if (!preg_match ( string $pattern , string $subject [, array &$matches [, int $flags = 0 [, int $offset = 0 ]]] ))
			return false;
	
			$line = $this->replaceFunction($line,"sum","+",$dim);
			$line = $this->replaceFunction($line,"allTrue","&&",$dim);
			$line = $this->replaceFunction($line,"oneTrue","||",$dim);
			$line = $this->replaceBinaryFunction($line,"min","vmin",$dim);
			$line = $this->replaceBinaryFunction($line,"max","vmax",$dim);*/
	}
	

	class CTemplateLoop
	{
		const User = 'User';
		const Functional = 'Functional';
		const Operator = 'Operator';
		const LogicOperator = 'LogicOperator';
		
		var $ar_CFunctionParameters=array();
		var $ar_CLines=array();
		var $key="";
		var $enAssemblyMode = AssemblyMode::Objects;
		var $strName = "";
		var $enType = CTemplateLoop::User;
		var $strTypeParameter = "";
		var $strReturnType = "";
		var $bClass = false;
		var $bFunction = false;
		var $bExport = true;
		var $strClassBase = '';
		var $strFunctionBase = '';
	}

	class CFunctionParameter
	{
		var $strName = "?";
		var $bIsVector = false;
		var $bIsReference = false;
		var $null_nDimensions = NULL;
		var $bIsConst = false;
		var $strComment = "";
		
		function toString()
		{
			return "<".$this->classify().$this->strName.">";
		}
		
		function classify()
		{
			return ($this->bIsConst?"const ":"").($this->bIsReference?'&':'').($this->bIsVector?"[".($this->null_nDimensions?$this->null_nDimensions:"*")."] ":"");
		}
		
		function generateDocumentation()
		{
			return "@param".($this->bIsConst || (!$this->bIsReference&&!$this->bIsVector)?"[in] ":"[out] ").$this->strName." ".$this->strComment;
			//return "<param name='".$this->strName."'>".($this->bIsConst || !$this->bIsVector?"[in] ":"[out] ").$this->strComment."</param>";
		}
		
		
		function	vtype($nDimensions, $type)
		{
			if ($this->null_nDimensions)
				$nDimensions = $this->null_nDimensions;
			return vtype($type,$nDimensions,$this->null_nDimensions!=NULL);
		}
		
		function assembleVaryingUnified($strScalarType, $iParameterIndex)
		{
			global $bUseDimensionConstant;
			/*if (!$bUseDimensionConstant)
				fatal("Trying to assemble varying unified parameter but bUseDimensionConstant is false");*/
			
			$strName = unifiedParameterName($iParameterIndex);
			if ($this->bIsVector)
			{
				global $enAssemblyMode;
				if ($enAssemblyMode == AssemblyMode::Objects)
					return ($this->bIsConst?"const ":"").$this->vtype(NULL,$strScalarType)."& $strName";
				if ($this->null_nDimensions)
					return ($this->bIsConst?"const ":"")."$strScalarType $strName"."[$this->null_nDimensions]";
				return ($this->bIsConst?"const ":"").($bUseDimensionConstant?"$strScalarType $strName".'[Dimensions]':"$strScalarType*$strName");
			}
			global $bUseConstReference;
			if ($bUseConstReference && !$this->bIsReference)
				return "const $strScalarType& $strName";
			return $strScalarType.($this->bIsReference?'&':'').' '.$strName;
		}
		
		function assemble($type, $nDimensions)
		{
			if ($this->bIsVector)
			{
				global $bUseDimensionConstant;
				global $enAssemblyMode;
				
				switch ($enAssemblyMode)
				{
					case AssemblyMode::Objects:
						return ($this->bIsConst?"const ":"").$this->vtype($nDimensions,$type)."& $this->strName";
					case AssemblyMode::Pointers:
						return ($this->bIsConst?"const ":"")."$type $this->strName"."[".($this->null_nDimensions?$this->null_nDimensions:($bUseDimensionConstant?"Dimensions":$nDimensions))."]";
					case AssemblyMode::DynamicPointers:
						if ($this->null_nDimensions)
							return ($this->bIsConst?"const ":"")."$type $this->strName"."[$this->null_nDimensions]";
						return ($this->bIsConst?"const ":"")."$type *$this->strName";
				}
				fatal("Unsupported assembly type ($enAssemblyMode)");
			}
			global $bUseConstReference;
			if ($bUseConstReference && !$this->bIsReference)
				return "const $type& $this->strName";
			return $type.($this->bIsReference?'&':'').' '.$this->strName;
		}
	};
	
	class CLine
	{
		var $content = "";
		var $indent = 0;
	};

	class CFunctionDefinition
	{
		var	$names = array("?");
		var	$parameters = array();
		var	$bHasVariableDimensions=false;
		var	$bHasAnyVectorParameters = false;
		var $bHasAnyScalarParameters = false;
		var $bCapableOfDirectRecursion = false;		//!< Indicates that the function (if implemented with $bUseDimensionConstant set true) can invoke itself instead of a helper class
		var	$return_type='void';
		var	$max_dimension=4;
		var $strComment = "";
		var $lines = array();
		var $float_type = NULL;
		var $general_constant_dimensions = NULL;
		var $bOnlyImplementForArrays = false;		//!< This function may only be implemented if processing array (not objects)
		var $return_type_comment = "";	//!< Comment to apply to return type definitions
		var $bMultipleReturns = false;	//!< Functions has more than one return point
		var $bExport = true;			//!< Function should be written to the out stream
		var $strNamespace = 'Vector';	//!< Namespace that this function is defined in
		
		
		function	keys()
		{
			$rs = array();
			foreach ($this->names as $strName)
				$rs[] = makeFunctionKey($strName,count($this->parameters));
			return $rs;
		}
		
		function	toString()
		{
			$pars = array();
			foreach ($this->parameters as $p)
			{
				if (gettype($p) == 'string')
					fatal("invalid parameter encountered of function $this->return_type ".implode("|",$this->names).": '$p' (string). Should be CFunctionParameter");
				array_push($pars,$p->toString());
			}
			return $this->return_type." ".implode("|",$this->names)." ".($this->bHasVariableDimensions?"(2..$this->max_dimension) ":"")."(".implode(", ",$pars).") direct=$this->bCapableOfDirectRecursion";
		}
		
		function	finalizeImport()
		{
			$this->bHasAnyScalarParameters = false;
			$this->bHasAnyVectorParameters = false;
			$this->bHasVariableDimensions = false;
			$this->general_constant_dimensions = NULL;
			$this->bMultipleReturns = false;
			$this->bCapableOfDirectRecursion = count($this->lines)>0 && preg_match('/^:iterate([^+]|$)/',$this->lines[0]->content);
			if (!count($this->lines))
				fatal("Function ".$this->toString()." has no lines\n");
				
			/*if (!$this->bCapableOfDirectRecursion)
			{
				echo $this->lines[0]->content."\n";
				echo "$this->bCapableOfDirectRecursion\n";
			}*/
			
			$returns = 0;
			$bFirst = true;
			foreach ($this->lines as $l)
			{
				if (!$bFirst && $this->bCapableOfDirectRecursion)
					$this->bCapableOfDirectRecursion = !preg_match('/^:iterate([^+]|$)/',$l->content);
				$bFirst= false;
				if (preg_match("/(^|[^a-zA-Z_0-9])return($|[^a-zA-Z_0-9])/",$l->content))
					$returns++;
			}
			
			//echo "$this->bCapableOfDirectRecursion\n";
			
			$this->bMultipleReturns = $returns > 1;
			
			foreach ($this->parameters as $p)
				if ($p->bIsVector)
				{
					$this->bHasAnyVectorParameters = true;
					if ($p->null_nDimensions)
					{
						if ($this->general_constant_dimensions === NULL)
							$this->general_constant_dimensions = $p->null_nDimensions;
						else
							if ($this->general_constant_dimensions != $p->null_nDimensions)
								$this->general_constant_dimensions = 0;
					}
					else
						$this->bHasVariableDimensions = true;
					
				}
				else
					$this->bHasAnyScalarParameters = true;
			
			$this->float_type = scalarType($this->parameters);
		}
		
		function	finalizeLine($line, $replace_returns_with,$variable_remapping,$direct_inline)
		{
			$rs = $line;
			if ($variable_remapping !== false)
			{
				if (count($variable_remapping) != count($this->parameters))
					fatal("Invalid variable remapping for function ".toString().". Function requires ".count($this->parameters)." parameters. Map, however, has ".count($variable_remapping));
				$i = 0;
				foreach($variable_remapping as $new_name)
				{
					if ($this->parameters[$i]->strName!= $new_name)
					{
						$rs = preg_replace("/(^|[^a-zA-Z_0-9.])".$this->parameters[$i]->strName."($|[^a-zA-Z_0-9])/",'${1}'.$new_name.'${2}',$rs);
						$rs = preg_replace("/(^|[^a-zA-Z_0-9.])".$this->parameters[$i]->strName."($|[^a-zA-Z_0-9])/",'${1}'.$new_name.'${2}',$rs);	//they overlap
					}
					//echo "after replacing '".$this->parameters[$i]->strName."' with '$new_name' rs is '$rs'\n";
					
					$i++;
				}
			}
		
			$rs = adjustVectorExpressions($rs);
			
			
			if ($replace_returns_with !== false)
				$rs = str_replace("return ",$replace_returns_with,$rs);
			if ($direct_inline)
				$rs = trim($rs,";\t ");
			return $rs;
		}
		
		function	iterateBlock($begin,$dim0,$dim,$extra_indent,$indent,$replace_returns_with,$variable_remapping,&$forwarding)
		{
			$end = $begin;
			while ($end < count($this->lines) && $this->lines[$end]->indent > $indent)
				$end++;
			
			global $bIterationTail;
			if ($begin == $end)
				return $end;
			
			if ($bIterationTail)
			{
				for ($j = $begin; $j < $end; $j++)
				{
					$ln = str_replace(":i",":Current",$this->lines[$j]->content);
					//echo "<".$ln.">\n";
					$ln = $this->finalizeLine($ln,$replace_returns_with,$variable_remapping,false);
					if ($ln != ";")
					{
						echo indent($extra_indent+$this->lines[$j]->indent-1).$ln."\n";
					}
				}
				return $end;
			}
			
			global $bUseDimensionConstant;
			global $enAssemblyMode;
			
			if ($enAssemblyMode == AssemblyMode::DynamicPointers)
			{
				echo indent($indent+$extra_indent)."for (register index_t iterator__=$dim0; iterator__ < dimensions; iterator__++)\n";
				echo indent($indent+$extra_indent)."{\n";
					for ($j = $begin; $j < $end; $j++)
					{
						echo indent($extra_indent+$this->lines[$j]->indent).$this->finalizeLine(str_replace(":i",":iterator__",$this->lines[$j]->content),$replace_returns_with,$variable_remapping,false);
						echo "\n";
					}
				echo indent($indent+$extra_indent)."}\n";
			}
			else if ($bUseDimensionConstant)
			{
				if ($this->bCapableOfDirectRecursion)
				{
					for ($j = $begin; $j < $end; $j++)
					{
						$ln = str_replace(":i",":Current",$this->lines[$j]->content);
						//echo "<".$ln.">\n";
						echo $this->finalizeLine(indent($extra_indent+$this->lines[$j]->indent-1).$ln,$replace_returns_with,$variable_remapping,false);
						echo "\n";
					}
				
				
					$call = classBase($this->strNamespace)."<Current+1,Dimensions,GreaterOrEqual<Current+2,Dimensions>::eval>::".$this->names[0]."(";
					
					if ($this->return_type != 'void')
						$call = "return $call";
					
					$bfirst = true;
					foreach($this->parameters as $p)
					{
						if (!$bfirst)
							$call .= ", ";
						$bfirst = false;
						$call .= $p->strName;
					}
					$call .= ")";
				}
				else
				{
					if (!$forwarding)
						fatal("Trying to iterate block with dimension constant being set, but forwarding is NULL");
					$lines = array_slice($this->lines,$begin,$end-$begin);
					makeUniform($lines,$forwarding);
					$f_name = requestIterationClass(CTemplateLoop::User,"",$this->names[0],$forwarding,$lines,$dim0,$this->return_type);
					foreach ($forwarding as &$f)
					{
						$bIsVector = $f->bIsVector;
						$f = $f->strName;
						if ($bIsVector && $enAssemblyMode == AssemblyMode::Objects)
							$f .=".v";
						
						/*if ($bIsVector && $dim0)
							$f .= "+$dim0";*/
					}
					
					//reduceObjectsToParameterValue($forwarding,"strName");
					$call = "$f_name(".implode(", ",$forwarding).")";
				}
				echo $this->finalizeLine(indent($indent+$extra_indent).$call,$replace_returns_with,$variable_remapping,false).";\n";
			}
			else
				for ($i = $dim0; $i < $dim; $i++)
				{
					for ($j = $begin; $j < $end; $j++)
					{
						echo $this->finalizeLine(indent($extra_indent+$this->lines[$j]->indent-1).str_replace(":i",":$i",$this->lines[$j]->content),$replace_returns_with,$variable_remapping,false);
						echo "\n";
					}
				}
			return $end;
		}
				
		function	resolve($line, $dim, $current_indent,$extra_indent)
		{
			
			while (replaceFunction($line,$dim, $this, $current_indent,$extra_indent));
			
			return $line;
		}
		
		function	implementHead($rs, $strName,$d,$fixed_dimensions)
		{
			global $bUseDimensionConstant;
			global $enAssemblyMode;
			
			
			$strDocBegin = '/**';
			$strSummaryEnd = '';
			$strDocEnd = '*/';
			$strDocBrief = '@brief ';
			$strDoc = '';
			$strReturnBegin = '@return ';
			$strReturnEnd = '';

			/*$strDocBegin = '///<summary>';
			$strDocEnd = '';
			$strSummaryEnd = '///</summary>';
			$strDocBrief = '///';
			$strDoc = '///';
			$strReturnBegin = '<returns>';
			$strReturnEnd = '</returns>';*/
			
			echo indent(0)."$strDocBegin\n";
			echo indent(1).$strDocBrief.nl2br(preg_replace('/<paramref>([^<]*)<\/paramref>/','@a \1',
					implode("\n".indent(1),explode("\n",$this->strComment))))."<br>\n".indent(1)."<br>\n";
			if (!$fixed_dimensions)
				echo indent(1).$strDoc."$d dimensional specialized version of ".implode("/",$this->names)."()<br>\n";
			if ($this->bHasAnyVectorParameters)
			{
				switch ($enAssemblyMode)
				{
					case AssemblyMode::Objects:
						echo indent(1).$strDoc."$strName() requires vector objects to operate on, rather than raw pointers. Use ref*() to create a temporary reference object to existing array pointers<br>\n";
					break;
					case AssemblyMode::Pointers:
					case AssemblyMode::DynamicPointers:
						echo indent(1).$strDoc."$strName() requires array pointers to operate on. Make sure all passed array pointers provide at least as many elements as required (";
					
						if ($this->general_constant_dimensions)
						{
							echo "$this->general_constant_dimensions";
							if (!$bUseDimensionConstant && $enAssemblyMode != AssemblyMode::DynamicPointers && $this->general_constant_dimensions != $d)
								echo "/$d";
						}
						else
							echo $bUseDimensionConstant?"<Dimensions>":($enAssemblyMode != AssemblyMode::DynamicPointers?"$d":"@a dimensions");
						echo " in general for this version)<br>\n";
					break;
				}
			}
			if ($this->bHasAnyScalarParameters)
			{
				global $bUseConstReference;
				if ($bUseConstReference)
					echo indent(1).$strDoc."Scalar values are passed by const reference rather than as copies. Use this version for complex types or if you are certain, your compiler is smart enough to distinguish.<br>\n";
				else
					echo indent(1).$strDoc."Scalar values are passed as copies rather than by const reference. Use this version for primitive types only.<br>\n";
			}
			echo indent(0)."$strSummaryEnd\n";
			
			for ($i = 0; $i < count($this->parameters); $i++)
				echo indent(1).$strDoc.$this->parameters[$i]->generateDocumentation()."\n";
			if ($enAssemblyMode == AssemblyMode::DynamicPointers)
				echo indent(1).$strDoc."@param dimensions [in] Number of dimensions to process\n";
			if ($rs != 'void')
				echo indent(1)."$strReturnBegin$this->return_type_comment $strReturnEnd\n";
			echo indent(0)."$strDocEnd\n";
			
			
			global $strBeforeFunctionType;
			global $strBeforeFunctionName;
			global $strBeforeFunctionBody;
			global $strStatic;
			echo indent(0).declareTemplates(count($this->parameters))."\n";
			echo indent(1)."$strStatic$strBeforeFunctionType\t$rs\t$strBeforeFunctionName\t$strName";
			switch ($enAssemblyMode)
			{
				case AssemblyMode::Objects:
				break;
				case AssemblyMode::Pointers:
					if (!$fixed_dimensions)
						echo $d;
				break;
				case AssemblyMode::DynamicPointers:
					echo "D";
				break;
			}
			echo "(";
			echo $this->parameters[0]->assemble(templateTypeName(0),$d);
			for ($i = 1; $i < count($this->parameters); $i++)
				echo ", ".$this->parameters[$i]->assemble(templateTypeName($i),$d);
			if ($enAssemblyMode == AssemblyMode::DynamicPointers)
				echo ", count_t dimensions";
			echo ")$strBeforeFunctionBody\n";
		}

		/**
			@brief Assembles a parameter configuration to be used to a template unroll helper class
		*/
		function createForwarding($strParameterSequence)
		{
			$ar_CFunctionParameter_out = array();
			$list = explode(",",$strParameterSequence);
			foreach ($list as $str)
			{
				$str = trim($str);
				$found = false;
				foreach ($this->parameters as $CFunctionParameter)
					if ($CFunctionParameter->strName == $str)
					{
						$ar_CFunctionParameter_out[] = $CFunctionParameter;
						$found = true;
						break;
					}
				if (!$found)
				{
					$CFunctionParameter = new CFunctionParameter();
					
					global $hmbDeclaredVectorVariables;
					$CFunctionParameter->strName = $str;
					if (array_key_exists($str,$hmbDeclaredVectorVariables))
					{
						$CFunctionParameter->bIsVector = true;
						$CFunctionParameter->strComment = "Forwarded temporary vector variable";
					}
					else
					{
						$CFunctionParameter->strComment = "Forwarded temporary scalar variable";
					}
					$ar_CFunctionParameter_out[] = $CFunctionParameter;
				}
			}
			return $ar_CFunctionParameter_out;
		}
		
		function implementInnerBody($d, $replace_returns_with,$variable_remapping,$extra_indent=0, $direct_inline=false)
		{
			//if ($direct_inline) echo '----';
			global $bUseDimensionConstant;
			global $bIterationHead;
			for ($l = 0; $l < count($this->lines); $l++)
			{
				$line = $this->lines[$l];
				$matches = array();
				if (preg_match("/^:iterate[\t ]*(\[(.+)\])*$/",$line->content,$matches))
				{
					if ($direct_inline)
						fatal("Can't directly inline function ".$this->toString().". Function uses :iterate");
					if ($bUseDimensionConstant)
						if (isset($matches[2]))
							$forward = $this->createForwarding($matches[2]);
						else
							$forward = $this->parameters;
					else
						$forward = NULL;
					$l = $this->iterateBlock($l+1,0,$d,$extra_indent,$line->indent,$replace_returns_with,$variable_remapping,$forward)-1;
				}
				else if (preg_match("/^:iterate\+[\t ]*(\[(.+)\])*$/",$line->content,$matches))
				{
					if ($direct_inline)
						fatal("Can't directly inline function ".$this->toString().". Function uses :iterate+");
						
					if ($bUseDimensionConstant)
						if (isset($matches[2]))
							$forward = $this->createForwarding($matches[2]);
						else
							$forward = $this->parameters;
					else
						$forward = NULL;
					$l = $this->iterateBlock($l+1,1,$d,$extra_indent,$line->indent,$replace_returns_with,$variable_remapping,$forward)-1;
				}
				else if (!$bIterationHead)
				{
					if (!$direct_inline)
						echo indent($extra_indent+$line->indent);
					echo $this->finalizeLine($this->resolve($line->content,$d,$line->indent,$direct_inline?0:$extra_indent),$replace_returns_with,$variable_remapping,$direct_inline);
					if (!$direct_inline)
						echo "\n";
				}
			}
		}
		
		function	returnType()
		{
			$t = $this->return_type;
			return trim(str_replace(":float", $this->float_type,$t));
		}
		
		function	implement()
		{
			global $bUseDimensionConstant;
			global $enAssemblyMode;
			
			
			if (!count($this->parameters))
				return;
			if ($this->bOnlyImplementForArrays && $enAssemblyMode == AssemblyMode::Objects)
				return;
			$rs = $this->returnType();
			
			beginFunctionImplementation();
			
			echo indent(0)."//now implementing template definition '".$this->toString()."'\n";
			if ($this->bHasVariableDimensions && !$bUseDimensionConstant && $enAssemblyMode != AssemblyMode::DynamicPointers)
				for ($d = 2; $d <= $this->max_dimension; $d++)
				{
					foreach ($this->names as $strName)
					{
						beginFunctionVarianceImplementation();
					
						$this->implementHead($rs,$strName,$d,false);
						echo indent(1)."{\n";
							$this->implementInnerBody($d, false,false);
						echo indent(1)."}\n\n";
					}
				}
			else
			{
				if (!$this->bHasVariableDimensions && $this->general_constant_dimensions)
					$d = $this->general_constant_dimensions;
				else
					$d = 3;
				foreach ($this->names as $strName)
					if (strlen($strName) > 1 || !$this->bHasVariableDimensions)
					{
						beginFunctionVarianceImplementation();
						
						$this->implementHead($rs,$strName,$d,true);
						echo indent(1)."{\n";
							$this->implementInnerBody($d,false,false);
						echo indent(1)."}\n\n";
					}
			}
			
			endFunctionImplementation($this->float_type);
		}
		
	};
	
	function import($strModule,$bPrimary=false)
	{
		global	$strBeforeFunctionType,
				$strBeforeFunctionName,
				$strBeforeFunctionBody,
				$strVector,
				$strOutFile,
				$bDataIsPrimary,
				$ar_CFunctionDefinitions,
				$hmiFunctionIndexTable,
				$strGlobalNamespace;
		
		//echo "$strModule...\n";
		$nWasPrimary = $bDataIsPrimary;
		$bDataIsPrimary = $bPrimary;
		
		ob_start();
			
			include("$strModule.template.php");
		
			$lines = explode("\n",ob_get_contents());
		ob_end_clean();
		err($strOutFile);
		
		//$bOldNamespace = $strGlobalNamespace;
		$strGlobalNamespace = $strNamespace;
		echo "<-$strNamespace\n";
		
		if ($lines === FALSE)
			die("Unable to open file");
		
		$ref_CFunctionDefinition_current = NULL;
		$enScanMode = ScanMode::Idle;		//Current scan mode
		
		foreach ($lines as $line)
		{
			$line = rtrim($line);
			//echo ">$line\n";
			if ($line == "config:")
			{
				$enScanMode = ScanMode::InConfig;
				continue;
			}
			if ($line == "functions:")
			{
				$enScanMode = ScanMode::InFunctions;
				continue;
			}
			if ($enScanMode == ScanMode::Idle)
				continue;
			
			if (!strlen($line))
			{
				if ($enScanMode == ScanMode::InFunctionComment || $enScanMode == ScanMode::InFunctionLines)
				{
					/*foreach ($ref_CFunctionDefinition_current->lines as &$l)
					{
						echo "\t[$l->indent]$l->content\n";
					}
					echo "\n";*/
					$ref_CFunctionDefinition_current->finalizeImport();
					unset($ref_CFunctionDefinition_current);
					$enScanMode = ScanMode::InFunctions;
					//return 0;
				}
				continue;
			}
			//echo ">$enScanMode>$line\n";
			$indent = strlen($line)-strlen(ltrim($line));
			/*$indent = 0;
			while ($indent < strlen($line) && $line[$indent] == '\t')
				$indent++;*/
			$line = substr($line,$indent);
			switch ($enScanMode)
			{
				case ScanMode::InFunctions:
				{
					$bOnlyImplementForArrays = (eraseSegmentIfBeginsWith($line,"arrays only: "));
					$matches = array();
					preg_match("/([a-zA-Z0-9_,]+)(<[0-9]+>)*\(([^\)]*)\)[ \t]*(->[ \t]*([^{ \t]+)[ \t]*({[^}]*})*)*/",$line,$matches);
					if (count($matches) <= 1)
					{
						echo "Unable to parse function definition '$line'\n";
						continue;
					}
						
					$f_index = count($ar_CFunctionDefinitions);
					array_push($ar_CFunctionDefinitions,new CFunctionDefinition());
					unset($ref_CFunctionDefinition_current);
					$ref_CFunctionDefinition_current = &$ar_CFunctionDefinitions[$f_index];
					
					$ref_CFunctionDefinition_current->bOnlyImplementForArrays = $bOnlyImplementForArrays;
					$ref_CFunctionDefinition_current->names = explode(',',$matches[1]);
					$ref_CFunctionDefinition_current->bExport = $bPrimary;
					$ref_CFunctionDefinition_current->strNamespace = $strNamespace;
					
					if ($matches[2])
						$ref_CFunctionDefinition_current->max_dimension = trim($matches[2],"<>");
					
					$pars = array();
					parseParameters($matches[3],$pars);						
					
					if (isset($matches[6]))
						$ref_CFunctionDefinition_current->return_type_comment = trim($matches[6],"{}");
					
					if (isset($matches[5]))
						$ref_CFunctionDefinition_current->return_type = $matches[5];
					else
					{
						echo "No return type in '$line'\n";
						print_r($matches);
						echo "\n";
					}
						

					
					$ref_CFunctionDefinition_current->parameters = array();
					
					foreach ($pars as $p)
					{
						$p = trim($p);
						$m = array();
						preg_match("/(const)*[ \t]*(\[([0-9]*)\]|&)*[ \t]*([a-zA-Z_][a-zA-Z_0-9]*)[ \t]*({.*})*/",$p,$m);
						array_push($ref_CFunctionDefinition_current->parameters,new CFunctionParameter());
						unset($par);
						$ref_CFunctionParameter = &$ref_CFunctionDefinition_current->parameters[count($ref_CFunctionDefinition_current->parameters)-1];
						$ref_CFunctionParameter->strName = $m[4];
						$ref_CFunctionParameter->bIsVector = strlen($m[2]) > 0 && $m[2] != '&';
						$ref_CFunctionParameter->bIsReference = $m[2] == '&';
						$ref_CFunctionParameter->null_nDimensions = $m[3]?$m[3]:NULL;//checking for empty string or 0, too, setting both to NULL
						$ref_CFunctionParameter->bIsConst = $m[1] == "const";
						if (isset($m[5]))
							$ref_CFunctionParameter->strComment = trim($m[5],"{}");
						
					}
					

					
					//echo $ref_CFunctionDefinition_current -> toString()."\n";
					
					$keys = $ref_CFunctionDefinition_current->keys();
					
					foreach($keys as $key)
						$hmiFunctionIndexTable[$key] = $f_index;
						
					
					$enScanMode = ScanMode::InFunctionComment;
					if (count($ar_CFunctionDefinitions) && $ar_CFunctionDefinitions[count($ar_CFunctionDefinitions)-1] == NULL)
						die ("NULL at ".__LINE__.": $line");
				}
				break;
				case ScanMode::InFunctionBlockComment:
					$strCommentLine = trim($line,"}");
					if (strlen($strCommentLine))
					{
						if (strlen($ref_CFunctionDefinition_current->strComment))
							$ref_CFunctionDefinition_current->strComment .= "\n";
						$ref_CFunctionDefinition_current->strComment .= $strCommentLine;
					}
					if ($line[strlen($line)-1] == '}')
					{
						$enScanMode = ScanMode::InFunctionLines;
						if (count($ar_CFunctionDefinitions) && $ar_CFunctionDefinitions[count($ar_CFunctionDefinitions)-1] == NULL)
							die ("NULL at ".__LINE__.": $line");
					}
				break;
				case ScanMode::InFunctionComment:
					if ($line[0] == '{' && $line[strlen($line)-1] == '}')
					{
						$ref_CFunctionDefinition_current->strComment = trim($line,"{}");
						$enScanMode = ScanMode::InFunctionLines;
						if (count($ar_CFunctionDefinitions) && $ar_CFunctionDefinitions[count($ar_CFunctionDefinitions)-1] == NULL)
							die ("NULL at ".__LINE__.": $line");
						break;
					}
					else
						if ($line[0] == '{')
						{
							$ref_CFunctionDefinition_current->strComment = trim($line,"{");
							$enScanMode = ScanMode::InFunctionBlockComment;
							if (count($ar_CFunctionDefinitions) && $ar_CFunctionDefinitions[count($ar_CFunctionDefinitions)-1] == NULL)
								die ("NULL at ".__LINE__.": $line");
							break;
						}
					$enScanMode = ScanMode::InFunctionLines;
				case ScanMode::InFunctionLines:
					array_push($ref_CFunctionDefinition_current->lines, new CLine());
					unset($l);
					$l = &$ref_CFunctionDefinition_current->lines[count($ref_CFunctionDefinition_current->lines)-1];
					$l->indent = $indent;
					$l->content = $line;
					if (count($ar_CFunctionDefinitions) && $ar_CFunctionDefinitions[count($ar_CFunctionDefinitions)-1] == NULL)
						die ("NULL at ".__LINE__.": $line");
					//print_r($ref_CFunctionDefinition_current->lines);
				break;
			};
		}
		
		if (isset($ref_CFunctionDefinition_current) && $ref_CFunctionDefinition_current)
		{
			$ref_CFunctionDefinition_current->finalizeImport();
			unset($ref_CFunctionDefinition_current);
		}	
	
		$bDataIsPrimary = $nWasPrimary;
		//$strGlobalNamespace = $bOldNamespace;
		
	}
	
	
	

	$strModule = isset($argv[1])?$argv[1]:'vector';

	import($strModule,true);
	
	/*foreach ($ar_CFunctionDefinitions as $f)
		echo $f->toString()."\n";*/
	
	
ob_start();
	

	echo "/*\n";
	echo "This file was generated from template definition '$strModule.template.php' on ".date("Y F jS H:i:s")."\nDo not edit\n";
	echo "*/\n";
	echo "\n\n";
	//print_r($ar_CFunctionDefinitions);
		/*echo "template <count_t Value0, count_t Value1>\n";
		echo "\tstruct GreaterOrEqual\n";
		echo "\t{\n";
		echo "\t\tstatic const bool eval=Value0>=Value1;\n";
		echo "\t};\n\n";*/

		$strStatic = '';
		echo "namespace $strGlobalNamespace\n{\n";
			$bUseDimensionConstant = false;
			$iGlobalIndent = 1;
			$bIterationTail = false;
			$bIterationHead = false;
			
			ob_start();
				$enAssemblyMode = AssemblyMode::DynamicPointers;
				echo "\n\t\t/* ----- Now dynamic_dimensions instances ----- */\n";
				foreach ($ar_CFunctionDefinitions as $f)
					if ($f->bHasVariableDimensions && $f->bExport)
						$f->implement();
				$out = ob_get_contents();
			ob_end_clean();
			
			foreach ($ar_CTemplateLoops as $cls)
			{
				if (!$cls->bFunction || !$cls->bExport)
					continue;
			
				echo indent(0).declareTemplates(count($cls->ar_CFunctionParameters))."\n";
				echo indent(1)."$strBeforeFunctionType	$cls->strReturnType	$strBeforeFunctionName	$cls->strName".'D(';
				$first = true;
				$p_index = 0;
				foreach ($cls->ar_CFunctionParameters as $p)
				{
					if (!$first)
						echo ", ";
					$first = false;
					echo $p->assembleVaryingUnified(templateTypeName($p_index),$p_index);
					$p_index++;
				}
				echo ', count_t dimensions';
				echo ")$strBeforeFunctionBody\n";
				
				$ar_CLines = $cls->ar_CLines;
				makeUniform($ar_CLines, $cls->ar_CFunctionParameters);
				
				echo indent(1)."{\n";
					switch ($cls->enType)
					{
						case CTemplateLoop::User:
						break;
						case CTemplateLoop::LogicOperator:
						case CTemplateLoop::Operator:
							echo indent(2)."$cls->strReturnType result = ".pointerAdjustDynamicVectorIteratorExpressions($ar_CLines[0]->content,0).";\n";
							echo indent(2)."for (register index_t iterator__ = 1; iterator__ < dimensions; iterator__++)\n";
							echo indent(3)."result = result $cls->strTypeParameter ".pointerAdjustDynamicVectorIteratorExpressions($ar_CLines[0]->content,'iterator__').";\n";
							echo indent(2)."return result;\n";
						break;
						case CTemplateLoop::Functional:
							echo indent(2)."$cls->strReturnType result = ".pointerAdjustDynamicVectorIteratorExpressions($ar_CLines[0]->content,0).";\n";
							echo indent(2)."for (register index_t iterator__ = 1; iterator__ < dimensions; iterator__++)\n";
							echo indent(3)."result = $cls->strTypeParameter(result,".pointerAdjustDynamicVectorIteratorExpressions($ar_CLines[0]->content,'iterator__').");\n";
							echo indent(2)."return result;\n";
						break;
					}
					
				
				echo indent(1)."}\n\n";
				

			}
			
			echo $out;
			$out = '';
			
			$enAssemblyMode = AssemblyMode::Objects;
			echo "\n\t\t/* ----- Now implementing fixed_dimensions instances ----- */\n";
			foreach ($ar_CFunctionDefinitions as $f)
				if ($f->bExport)
					$f->implement();

					
					
		echo "};\n\n";
					
	
		$strStatic = "static\t";
		echo "template <count_t Current, count_t Dimensions, bool Terminal>\n";
		echo "\tclass ".classBase()."\n\t{};\n";
		echo "template <count_t Current, count_t Dimensions>\n";
		echo "\tclass ".classBase()."<Current,Dimensions,true>\n\t{\n\tpublic:\n";
			$bUseDimensionConstant = true;
			$enAssemblyMode = AssemblyMode::Pointers;
			$iGlobalIndent = 2;
			$bIterationTail = true;
			$bIterationHead = false;
			echo "\n\t\t/* ----- Now implementing pointer based recursion terminators ----- */\n";
			foreach ($ar_CFunctionDefinitions as $f)
				if ($f->bHasVariableDimensions && $f->bCapableOfDirectRecursion && $f->bExport)
					$f->implement();
			echo "\n\t\t/* ----- Now implementing object based recursion terminators ----- */\n";
			$enAssemblyMode = AssemblyMode::Objects;
			foreach ($ar_CFunctionDefinitions as $f)
				if ($f->bHasVariableDimensions && $f->bCapableOfDirectRecursion && $f->bExport)
					$f->implement();
	
	ob_start();
			$bUseDimensionConstant = true;
			$iGlobalIndent = 2;
			$bIterationTail = false;
			$bIterationHead = true;
			
			echo "\n\t\t/* ----- Now implementing pointer based recursion terminators ----- */\n";
			$enAssemblyMode = AssemblyMode::Pointers;
			foreach ($ar_CFunctionDefinitions as $f)
				if ($f->bHasVariableDimensions && $f->bCapableOfDirectRecursion && $f->bExport)
					$f->implement();
			echo "\n\t\t/* ----- Now implementing object based recursion terminators ----- */\n";
			$enAssemblyMode = AssemblyMode::Objects;
			foreach ($ar_CFunctionDefinitions as $f)
				if ($f->bHasVariableDimensions && $f->bCapableOfDirectRecursion && $f->bExport)
					$f->implement();

			$bIterationTail = false;
			$bIterationHead = false;
					
			echo "\n\t\t/* ----- Now implementing pointer based recursion-indirect functions ----- */\n";
			$enAssemblyMode = AssemblyMode::Pointers;
			foreach ($ar_CFunctionDefinitions as $f)
				if ($f->bHasVariableDimensions && !$f->bCapableOfDirectRecursion && $f->bExport)
					$f->implement();
			echo "\n\t\t/* ----- Now implementing object based recursion-indirect functions ----- */\n";
			$enAssemblyMode = AssemblyMode::Objects;
			foreach ($ar_CFunctionDefinitions as $f)
				if ($f->bHasVariableDimensions && !$f->bCapableOfDirectRecursion && $f->bExport)
					$f->implement();

		echo "\t};\n\n";
		
		
		echo "template <count_t Dimensions, count_t First=0>\n";
		echo "\tclass $strGlobalNamespace"."Unroll:public ".classBase()."<First,Dimensions,GreaterOrEqual<First+1,Dimensions>::eval> \n\t{};\n";
	
		$out = ob_get_contents();
	ob_end_clean();
	
	echo "\n\t\t/* ----- Now implementing helper class terminators ----- */\n";
	
		$enAssemblyMode = AssemblyMode::Pointers;
		foreach ($ar_CTemplateLoops as $cls)
		{
			if (!$cls->bClass || !$cls->bExport)
				continue;
		
			echo indent(0).declareTemplates(count($cls->ar_CFunctionParameters))."\n";
			echo indent(1)."static $strBeforeFunctionType	$cls->strReturnType	$strBeforeFunctionName	$cls->strName(";
			$first = true;
			$p_index = 0;
			foreach ($cls->ar_CFunctionParameters as $p)
			{
				if (!$first)
					echo ", ";
				$first = false;
				echo $p->assembleVaryingUnified(templateTypeName($p_index),$p_index);
				$p_index++;
			}
			echo ")$strBeforeFunctionBody\n";
			
			$ar_CLines = $cls->ar_CLines;
			makeUniform($ar_CLines, $cls->ar_CFunctionParameters);
			
			echo indent(1)."{\n";
				switch ($cls->enType)
				{
					case CTemplateLoop::User:
						foreach ($ar_CLines as $l)
							echo indent($l->indent-1).pointerAdjustVectorIteratorExpressions($l->content)."\n";
					break;
					case CTemplateLoop::LogicOperator:
					case CTemplateLoop::Operator:
					case CTemplateLoop::Functional:
						$l = $ar_CLines[0];
						echo indent($l->indent+2);
						echo "return ";
						echo pointerAdjustVectorIteratorExpressions($l->content).";\n";
					break;
				}
				
			
			echo indent(1)."}\n\n";
			

		}
		echo "\t};\n\n";
	

		echo "template <count_t Current, count_t Dimensions>\n";
		echo "\tclass ".classBase()."<Current,Dimensions,false>\n\t{\n\tpublic:\n";
	
		echo "\n\t\t/* ----- Now implementing helper class iterators ----- */\n";
	
		$enAssemblyMode = AssemblyMode::Pointers;
		foreach ($ar_CTemplateLoops as $cls)
		{
			if (!$cls->bClass || !$cls->bExport)
				continue;
			echo "\n\t\t/* --- Now processing '$cls->strKey' --- */\n";
			echo indent(0).declareTemplates(count($cls->ar_CFunctionParameters))."\n";
			echo indent(1)."static $strBeforeFunctionType	$cls->strReturnType	$strBeforeFunctionName	$cls->strName(";
			$first = true;
			$p_index = 0;
			foreach ($cls->ar_CFunctionParameters as $p)
			{
				if (!$first)
					echo ", ";
				if (gettype($p) == 'string')
					fatal("invalid parameter encountered of function $cls->strReturnType $cls->strName: '$p' (string). Should be CFunctionParameter");
					
				$first = false;
				echo $p->assembleVaryingUnified(templateTypeName($p_index),$p_index);
				$p_index++;
			}
			echo ")$strBeforeFunctionBody\n";
			$ar_CLines = $cls->ar_CLines;
			makeUniform($ar_CLines, $cls->ar_CFunctionParameters);
			
			echo indent(1)."{\n";
				switch ($cls->enType)
				{
					case CTemplateLoop::User:
						foreach ($ar_CLines as $l)
							echo indent($l->indent-1).pointerAdjustVectorIteratorExpressions($l->content)."\n";
						echo indent(2);
						if ($cls->strReturnType != 'void')
							echo "return ";
						echo $cls->strClassBase."<Current+1,Dimensions, GreaterOrEqual<Current+2,Dimensions>::eval>::$cls->strName(".unifiedParameters(count($cls->ar_CFunctionParameters)).");\n";
					break;
					case CTemplateLoop::LogicOperator:
					case CTemplateLoop::Operator:
						echo indent(2)."return (\n";
						$l = $ar_CLines[0];
						echo indent(4).pointerAdjustVectorIteratorExpressions($l->content)." $cls->strTypeParameter\n";
						//echo indent(4)."$strVectorBaseClass<Current,(Current+Dimensions)/2, GreaterOrEqual<Current+1,(Current+Dimensions)/2> >::$cls->strName(".implode_p(", ",$cls->ar_CFunctionParameters,"strName").") $cls->strTypeParameter\n";
						//echo indent(4)."$strVectorBaseClass<(Current+Dimensions)/2,Dimensions, GreaterOrEqual<(Current+Dimensions)/2+1,Dimensions> >::$cls->strName(".implode_p(", ",$cls->ar_CFunctionParameters,"strName").")\n";
						echo indent(4).$cls->strClassBase."<Current+1,Dimensions, GreaterOrEqual<Current+2,Dimensions>::eval>::$cls->strName(".unifiedParameters(count($cls->ar_CFunctionParameters)).")\n";
						echo indent(3).");\n";
					break;
					case CTemplateLoop::Functional:
						echo indent(2)."return $cls->strTypeParameter(\n";
						$l = $ar_CLines[0];
						echo indent(4).pointerAdjustVectorIteratorExpressions($l->content).",\n";
						
						//echo indent(4)."$strVectorBaseClass<Current,(Current+Dimensions)/2, GreaterOrEqual<Current+1,(Current+Dimensions)/2> >::$cls->strName(".implode_p(", ",$cls->ar_CFunctionParameters,"strName")."),\n";
						echo indent(4).$cls->strClassBase."<Current+1,Dimensions, GreaterOrEqual<Current+2,Dimensions>::eval>::$cls->strName(".unifiedParameters(count($cls->ar_CFunctionParameters)).")\n";
						echo indent(3).");\n";
					break;
				}
			echo indent(1)."}\n\n";
		}
		
	
	echo $out;
	
	$data = ob_get_contents();
ob_end_clean();
	
	$file = fopen($strOutFile.'.h','w+');
	if (!$file)
		fatal("Unable to open file $strOutFile.h for output");
	
	fwrite($file,"#ifndef $strOutFile"."H\n#define $strOutFile"."H\n");
	fwrite($file,$data);
	fwrite($file,"\n#endif\n");
	fclose($file);
	echo "$strOutFile.h updated\n";
?>