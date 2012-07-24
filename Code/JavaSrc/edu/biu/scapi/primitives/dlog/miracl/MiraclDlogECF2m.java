package edu.biu.scapi.primitives.dlog.miracl;

import java.io.IOException;
import java.math.BigInteger;
import java.util.Properties;

import org.bouncycastle.util.encoders.Hex;

import edu.biu.scapi.primitives.dlog.DlogECF2m;
import edu.biu.scapi.primitives.dlog.ECElement;
import edu.biu.scapi.primitives.dlog.ECF2mUtility;
import edu.biu.scapi.primitives.dlog.GroupElement;
import edu.biu.scapi.primitives.dlog.groupParams.ECF2mGroupParams;
import edu.biu.scapi.primitives.dlog.groupParams.ECF2mKoblitz;
import edu.biu.scapi.primitives.dlog.groupParams.ECF2mPentanomialBasis;
import edu.biu.scapi.primitives.dlog.groupParams.ECF2mTrinomialBasis;
import edu.biu.scapi.primitives.dlog.groupParams.ECFpGroupParams;
import edu.biu.scapi.primitives.dlog.groupParams.GroupParams;
import edu.biu.scapi.securityLevel.DDH;

/**
 * This class implements a Dlog group over F2m utilizing Miracl++'s implementation.<p>
 * It uses JNI technology to call Miracl's native code.
 * @author Cryptography and Computer Security Research Group Department of Computer Science Bar-Ilan University (Moriya Farbstein)
 */
public class MiraclDlogECF2m extends MiraclAdapterDlogEC implements DlogECF2m, DDH{

	private native void initF2mCurve(long mip, int m, int k1, int k2, int k3, byte[] a, byte[] b);
	private native long multiplyF2mPoints(long mip, long p1, long p2);
	private native long simultaneousMultiplyF2m(long mip, long[] points, byte[][] exponents);
	private native long exponentiateF2mPoint(long mip, long p, byte[] exponent);
	private native long invertF2mPoint(long mip, long p);
	private native boolean validateF2mGenerator(long mip, long generator, byte[] x, byte[] y);
	private native boolean isF2mMember(long mip, long point);
	private native long createInfinityF2mPoint(long mip);
	private native long createECF2mObject(long mip, int m, int k1, int k2, int k3, byte[] a, byte[] b);
	private native long exponentiateF2mWithPreComputed(long mip, long dlogGroup, long base, byte[] size, int window, int maxBits);
	
	private long nativeDlog = 0;
	private ECF2mUtility util;
	
	/**
	 * Default constructor. Initializes this object with K-163 NIST curve.
	 */
	public MiraclDlogECF2m() throws IOException {
		this("K-163");
	}

	public MiraclDlogECF2m(String fileName, String curveName) throws IOException{
		super(fileName, curveName);
	}
	/**
	 * Initialize this DlogGroup with one of NIST recommended elliptic curve
	 * @param curveName - name of NIST curve to initialized
	 * @throws IOException 
	 * @throws IllegalAccessException
	 */
	public MiraclDlogECF2m(String curveName) throws IllegalArgumentException, IOException{
		this(PROPERTIES_FILES_PATH, curveName);
	}

	/**
	 * Extracts the parameters of the curve from the properties object and initialize the groupParams, 
	 * generator and the underlying curve
	 * @param ecProperties - properties object contains the curve file data
	 * @param curveName - the curve name as it called in the file
	 */
	protected void doInit(Properties ecProperties, String curveName) {
		util = new ECF2mUtility();
		groupParams = util.checkAndCreateInitParams(ecProperties, curveName);
		
		createUnderlyingCurveAndGenerator();
		
	}
	
	private void createUnderlyingCurveAndGenerator(){
		BigInteger x;
		BigInteger y;
		if(groupParams instanceof ECF2mTrinomialBasis){
			ECF2mTrinomialBasis triParams = (ECF2mTrinomialBasis)groupParams;
			int k2 = 0;
			int k3 = 0;
			initF2mCurve(getMip(), triParams.getM(), triParams.getK1(), k2, k3, triParams.getA().toByteArray(), triParams.getB().toByteArray());
			x = triParams.getXg();
			y = triParams.getYg();
		}else{
			//we assume that if it's not trinomial then it's pentanomial. We do not check.
			ECF2mPentanomialBasis pentaParams = (ECF2mPentanomialBasis) groupParams;
			
			initF2mCurve(getMip(), pentaParams.getM(), pentaParams.getK3(), pentaParams.getK2(), pentaParams.getK1(), pentaParams.getA().toByteArray(), pentaParams.getB().toByteArray());
			x = pentaParams.getXg();
			y = pentaParams.getYg();
		}
			
		// create the generator
		// here we assume that (x,y) are the coordinates of a point that is indeed a generator
		generator = new ECF2mPointMiracl(x, y, this);
	}

	/**
	 * @return the type of the group - ECF2m
	 */
	public String getGroupType() {
		return util.getGroupType();
	}

	/**
	 * Calculates the inverse of the given GroupElement
	 * @param groupElement to inverse
	 * @return the inverse element of the given GroupElement
	 * @throws IllegalArgumentException
	 */
	public GroupElement getInverse(GroupElement groupElement) throws IllegalArgumentException{
		
		//if the GroupElement doesn't match the DlogGroup, throw exception
		if (!(groupElement instanceof ECF2mPointMiracl)){
			throw new IllegalArgumentException("groupElement doesn't match the DlogGroup");
		}

		// the inverse of infinity point is infinity
		if (((ECF2mPointMiracl) groupElement).isInfinity()) {
			return groupElement;
		}

		long point = ((ECF2mPointMiracl) groupElement).getPoint();
		// call to native inverse function
		long result = invertF2mPoint(mip, point);
		// build a ECF2mPointMiracl element from the result value
		return new ECF2mPointMiracl(result, this);

	}

	/**
	 * Multiplies two GroupElements
	 * @param groupElement1
	 * @param groupElement2
	 * @return the multiplication result
	 * @throws IllegalArgumentException
	 */
	public GroupElement multiplyGroupElements(GroupElement groupElement1,
			GroupElement groupElement2) throws IllegalArgumentException {

		// if the GroupElements don't match the DlogGroup, throw exception
		if (!(groupElement1 instanceof ECF2mPointMiracl)) {
			throw new IllegalArgumentException("groupElement doesn't match the DlogGroup");
		}
		if (!(groupElement2 instanceof ECF2mPointMiracl)){
			throw new IllegalArgumentException("groupElement doesn't match the DlogGroup");
		}

		//if one of the points is the infinity point, the second one is the multiplication result
		if (((ECF2mPointMiracl) groupElement1).isInfinity()) {
			return groupElement2;
		}
		if (((ECF2mPointMiracl) groupElement2).isInfinity()) {
			return groupElement1;
		}

		long point1 = ((ECF2mPointMiracl) groupElement1).getPoint();
		long point2 = ((ECF2mPointMiracl) groupElement2).getPoint();

		// call to native multiply function
		long result = multiplyF2mPoints(mip, point1, point2);
		// build a ECF2mPointMiracl element from the result value
		return new ECF2mPointMiracl(result, this);

	}

	/**
	 * Calculates the exponentiate of the given GroupElement
	 * @param exponent
	 * @param base
	 * @return the result of the exponentiation
	 * @throws IllegalArgumentException
	 */
	public GroupElement exponentiate(GroupElement base, BigInteger exponent) 
									 throws IllegalArgumentException{
		
		//if the GroupElements don't match the DlogGroup, throw exception
		if (!(base instanceof ECF2mPointMiracl)){
			throw new IllegalArgumentException("groupElement doesn't match the DlogGroup");
		}

		// infinity remains the same after any exponentiate
		if (((ECF2mPointMiracl) base).isInfinity()) {
			return base;
		}

		long point = ((ECF2mPointMiracl) base).getPoint();
		// call to native exponentiate function
		long result = exponentiateF2mPoint(mip, point, exponent.toByteArray());
		// build a ECF2mPointMiracl element from the result value
		return new ECF2mPointMiracl(result, this);

	}

	/**
	 * Computes the product of several exponentiations with distinct bases 
	 * and distinct exponents. 
	 * Instead of computing each part separately, an optimization is used to 
	 * compute it simultaneously. 
	 * @param groupElements
	 * @param exponentiations
	 * @return the exponentiation result
	 */
	@Override
	public GroupElement simultaneousMultipleExponentiations(GroupElement[] groupElements, 
			BigInteger[] exponentiations) {
		
		//Koblitz curve has an optimization that causes the naive algorithm to be faster than the following optimized algorithm.
		//so currently we use the naive algorithm instead of the optimized algorithm.
		// may be in the future this will be change.
		if (groupParams instanceof ECF2mKoblitz) {
			return computeNaive(groupElements, exponentiations);
		}

		int len = groupElements.length;

		//Our test results show that for elliptic curve over F2m and n<60 the naive algorithm gives the best performances
		if (len < 60) {
			return computeNaive(groupElements, exponentiations);
		}

		long[] nativePoints = new long[len];
		byte[][] exponents = new byte[len][];
		for (int i = 0; i < len; i++) {
			// if the GroupElements don't match the DlogGroup, throw exception
			if (!(groupElements[i] instanceof ECF2mPointMiracl)) {
				throw new IllegalArgumentException("groupElement doesn't match the DlogGroup");
			}
			nativePoints[i] = ((ECF2mPointMiracl) groupElements[i]).getPoint();
			exponents[i] = exponentiations[i].toByteArray();
		}

		// call to native exponentiate function
		long result = simultaneousMultiplyF2m(mip, nativePoints, exponents);
		// build a ECF2mPointMiracl element from the result value
		return new ECF2mPointMiracl(result, this);
	}

	@Override
	public GroupElement exponentiateWithPreComputedValues
			(GroupElement groupElement, BigInteger exponent){
		
		//if the GroupElements don't match the DlogGroup, throw exception
		if (!(groupElement instanceof ECF2mPointMiracl)){
			throw new IllegalArgumentException("groupElement doesn't match the DlogGroup");
		}

		ECF2mPointMiracl base = (ECF2mPointMiracl) groupElement;

		// infinity remains the same after any exponentiate
		if (base.isInfinity()) {
			return base;
		}

		if (nativeDlog == 0) {

			int m, k1 = 0, k2 = 0, k3 = 0;
			boolean trinomial = false;
			ECF2mGroupParams params = (ECF2mGroupParams) groupParams;
			BigInteger a, b;
			m = params.getM();
			a = params.getA();
			b = params.getB();
			// create the curve
			if (params instanceof ECF2mKoblitz) {
				params = ((ECF2mKoblitz) params).getCurve();
			}
			if (params instanceof ECF2mTrinomialBasis) {
				ECF2mTrinomialBasis paramsT = (ECF2mTrinomialBasis) params;
				k1 = paramsT.getK1();
				trinomial = true;
			} else if (params instanceof ECF2mPentanomialBasis) {
				ECF2mPentanomialBasis paramsP = (ECF2mPentanomialBasis) params;
				k1 = paramsP.getK1();
				k2 = paramsP.getK2();
				k3 = paramsP.getK3();
				trinomial = false;
			}

			if (trinomial) {
				nativeDlog = createECF2mObject(mip, m, k1, 0, 0, a.toByteArray(), b.toByteArray());
			} else{
				nativeDlog = createECF2mObject(mip, m, k3, k2, k1, a.toByteArray(), b.toByteArray());
			}
		}

		// call to native exponentiate function
		long result = exponentiateF2mWithPreComputed(mip, nativeDlog, base.getPoint(), exponent.toByteArray(), getWindow(), getOrder().bitLength());

		// build a ECF2mPointMiracl element from the result value
		return new ECF2mPointMiracl(result, this);
	}

	/**
	 * Creates a point in the F2m field with the given parameters
	 * 
	 * @return the element
	 */
	public ECElement generateElement(BigInteger x, BigInteger y) throws IllegalArgumentException{
		//Creates element with the given values.
		ECF2mPointMiracl point =  new ECF2mPointMiracl(x, y, this);
		
		//if the element was created, it is a point on the curve.
		//checks if the point is in the sub-group, too.
		boolean valid = util.checkSubGroupMembership(this, point);
		
		//if the point is not in the sub-group, throw exception.
		if (valid == false){
			throw new IllegalArgumentException("Could not generate the element. The given (x, y) is not a point in this Dlog group");
		}
		
		return point;
	}

	/**
	 * Check if the given element is member of this Dlog group
	 * @param element
	 * @return true if the given element is member of that group. false, otherwise.
	 * @throws IllegalArgumentException
	 */
	public boolean isMember(GroupElement element) {

		// checks that the element is the correct object
		if (!(element instanceof ECF2mPointMiracl)) {
			throw new IllegalArgumentException("groupElement doesn't match the DlogGroup");
		}

		ECF2mPointMiracl point = (ECF2mPointMiracl) element;
		// infinity point is a valid member
		if (point.isInfinity()) {
			return true;
		}

		// A point (x, y) is a member of a Dlog group with prime order q over an Elliptic Curve if it meets the following two conditions:
		// 1)	P = (x,y) is a point in the Elliptic curve, i.e (x,y) is a solution of the curve�s equation.
		// 2)	P = (x,y) is a point in the q-order group which is a sub-group of the Elliptic Curve.
		// those two checks is done in two steps:
		// 1.	Checking that the point is on the curve, performed by checkCurveMembership
		// 2.	Checking that the point is in the Dlog group,performed by checkSubGroupMembership

		boolean valid = util.checkCurveMembership((ECF2mGroupParams) groupParams, point.getX(), point.getY());
		valid = valid && util.checkSubGroupMembership(this, point);
		
		return valid;
	}

	public ECElement getInfinity() {
		long infinity = createInfinityF2mPoint(mip);
		return new ECF2mPointMiracl(infinity, this);
	}

	/**
	 * Encode a byte array to an ECF2mPointBc. Some constraints on the byte array are necessary so that it maps into an element of this group.
	 * Currently we don't support this conversion. It will be implemented in the future.Meanwhile we return null.
	 * @param binaryString the byte array to convert
	 * @return the created group Element
	 */
	public GroupElement encodeByteArrayToGroupElement(byte[] binaryString) {
		//currently we don't support this conversion. 
		//will be implemented in the future.
		return null;
	}

	/**
	 * Decode an ECF2mPointBc that was obtained through the encodeByteArrayToGroupElement function to the original byte array.
	 * Currently we don't support this conversion. It will be implemented in the future.Meanwhile we return null.
	 * @param groupElement the element to convert
	 * @return the created byte array
	 */
	public byte[] decodeGroupElementToByteArray(GroupElement groupElement) {
		if (!(groupElement instanceof ECF2mPointMiracl)) {
			throw new IllegalArgumentException("element type doesn't match the group type");
		}
		//currently we don't support this conversion. 
		//will be implemented in the future.
		return null;
	}

	
	/**
	 * This is 1-1 mapping of any element of this group to a byte array representation.
	 * Currently we don't support this conversion. It will be implemented in the future.Meanwhile we return null.
	 * @param groupElement the element to convert
	 * @return the byte array representation
	 */
	public byte[] mapAnyGroupElementToByteArray(GroupElement groupElement) {
		// TODO Auto-generated method stub
		return null;
	}
	// upload MIRACL library
		static {
			System.loadLibrary("MiraclJavaInterface");
		}
}
